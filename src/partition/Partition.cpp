#include <algorithm>
#include "Partition.h"
#include "../hungarian.h"
#include "../Defines.h"

Partition::Partition(int n) : nnode(n), pbkt_size(1) {
    allocate(n);
}

Partition::~Partition() {
    deallocate();
}

void Partition::allocate(int n) {
    ppos = new int[n + 1];
    pbkt = new int[n + 1];
    pcnt = new int[n + 1];
    pvertex = new int[n];
    // assert(n == 500);
}

void Partition::deallocate() {
    delete[] ppos;
    delete[] pbkt;
    delete[] pcnt;
    delete[] pvertex;
}

Partition::Partition(const Partition& other)
    : nnode(other.getNnode()) {
    allocate(nnode);
    copyPartition(other);
}

Partition& Partition::operator=(const Partition& other) {
    if (this != &other) {
        deallocate();
        nnode = other.getNnode();
        copyPartition(other);
    }
    return *this;
}

void Partition::buildPartition(int* vpart) {
    for (int i = 0; i < nnode + 1; ++i) {
        ppos[i] = i;
        pbkt[i] = i;
    }
    pbkt_size = 1;
    std::memset(pcnt, 0, sizeof(int) * (nnode + 1));
    std::memset(pvertex, -1, sizeof(int) * nnode);

    for (int i = 0; i < nnode; ++i) {
        int pid = vpart[i];
        if (pid == EMPTY_IDX || pid > nnode) {
            printf("Invalid partition for node %d\n", pid);
            exit(0);
        }
        if (ppos[pid] >= pbkt_size) {
            if (pbkt_size == nnode + 1) {
                printf("The bucket is full, no new partition could be added\n");
                exit(0);
            }
            int end_pid = pbkt[pbkt_size];
            std::swap(pbkt[pbkt_size], pbkt[ppos[pid]]);
            std::swap(ppos[end_pid], ppos[pid]);
            pbkt_size++;
            pcnt[pid]++;
        }
        else {
            pcnt[pid]++;
        }
    }
    std::memcpy(pvertex, vpart, sizeof(int) * nnode);
}

void Partition::copyPartition(const Partition& source) {
    pbkt_size = source.pbkt_size;
    if (nnode != source.nnode) {
        printf("Partitions are incompatible and cannot be copied.\n");
        exit(0);
    }

    std::memcpy(pbkt, source.pbkt, sizeof(int) * (nnode + 1));
    std::memcpy(pcnt, source.pcnt, sizeof(int) * (nnode + 1));
    std::memcpy(ppos, source.ppos, sizeof(int) * (nnode + 1));
    std::memcpy(pvertex, source.pvertex, sizeof(int) * nnode);
    
    value = source.getValue();
}

void swapAry(int* ary, int idx1, int idx2) {
    int tmp = ary[idx1];
    ary[idx1] = ary[idx2];
    ary[idx2] = tmp;
}

void Partition::updatePartition(int v, int target) {
    int oldpartition = pvertex[v];
    assert(target != EMPTY_IDX);
    if (oldpartition == target)
        printf("Meaningless move\n");
    pcnt[oldpartition]--;
    pcnt[target]++;
    if (pcnt[oldpartition] == 0) {
        int end_pid = pbkt[pbkt_size - 1];
        swapAry(pbkt, pbkt_size - 1, ppos[oldpartition]);
        swapAry(ppos, end_pid, oldpartition);
        pbkt_size--;
    }
    pvertex[v] = target;
}

void Partition::print(FILE* fout) const {
    for (int i = 0; i < nnode; ++i) {
        fprintf(fout, "%d ", pvertex[i]);
    }
    fprintf(fout, "\n");
}

int Partition::calculateDistance(const int* p1, const int* p2) const {
    int sum = 0;
    for (int i = 0; i < nnode; ++i) {
        for (int j = i + 1; j < nnode; ++j) {
            if (p1[i] == p1[j]) {
                if (p2[i] != p2[j])
                    sum++;
            }
            else {
                if (p2[i] == p2[j])
                    sum++;
            }
        }
    }
    return sum;
}

int Partition::calculateScore(int** matrix) {
    int score = 0;
    const int* pvertex = getPvertex();

    for (int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            if (pvertex[i] == pvertex[j]) {
                score += matrix[i][j];
            }
        }
    }

    return score;
}


void Partition::setValue(int new_value) {
    value = new_value;
}

int Partition::getValue() const {
    return value;
}

int* Partition::getPvertex() const {
    return pvertex;
}

int* Partition::getPcount() {
    return pcnt;
}

void Partition::setBucketSize(int bucket_size) {
    pbkt_size = bucket_size;
}

int Partition::getBucketSize() const {
    return pbkt_size;
}

int* Partition::getBucket() {
    return pbkt;
}

int Partition::getNnode() const {
    return nnode;
}

int Partition::sizeIntersection(const std::vector<int>& v1, const std::vector<int>& v2) const {
    std::vector<int> sorted_v1 = v1;
    std::vector<int> sorted_v2 = v2;
    std::sort(sorted_v1.begin(), sorted_v1.end());
    std::sort(sorted_v2.begin(), sorted_v2.end());

    std::vector<int> intersection_result((std::min)(sorted_v1.size(), sorted_v2.size()));

    auto it = std::set_intersection(
        sorted_v1.begin(), sorted_v1.end(),
        sorted_v2.begin(), sorted_v2.end(),
        intersection_result.begin()
    );

    intersection_result.resize(it - intersection_result.begin());

    return intersection_result.size();
}

int Partition::calculateMaxMatch(int* p1, int n_part1, int* p2, int n_part2) const {
    std::vector<std::vector<int>> group1(n_part1);
    std::vector<std::vector<int>> group2(n_part2);
    int* par2idx = new int[nnode + 1];
    std::memset(par2idx, -1, sizeof(int) * (nnode + 1));

    int order_cnt = 0;
    for (int i = 0; i < nnode; i++) {
        if (par2idx[p1[i]] == -1) {
            par2idx[p1[i]] = order_cnt++;
        }
        group1[par2idx[p1[i]]].push_back(i);
    }
    assert(order_cnt == n_part1);

    std::memset(par2idx, -1, sizeof(int) * (nnode + 1));
    order_cnt = 0;
    for (int i = 0; i < nnode; ++i) {
        if (par2idx[p2[i]] == -1) {
            par2idx[p2[i]] = order_cnt++;
        }
        group2[par2idx[p2[i]]].push_back(i);
    }
    assert(order_cnt == n_part2);

    int row = max(n_part1, n_part2);
    int** cost_mat = new int* [row];
    for (int i = 0; i < row; i++) {
        cost_mat[i] = new int[row]();
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < row; j++) {
            if (i < n_part1 && j < n_part2) {
                cost_mat[i][j] = sizeIntersection(group1[i], group2[j]);
            }
        }
    }

    hungarian_problem_t p;
    int matrix_size = hungarian_init(&p, cost_mat, row, row, HUNGARIAN_MODE_MAXIMIZE_UTIL);
    hungarian_solve(&p);

    int sum = 0;
    for (int i = 0; i < matrix_size; ++i) {
        for (int j = 0; j < matrix_size; ++j) {
            sum += cost_mat[i][j] * p.assignment[i][j];
        }
    }

    hungarian_free(&p);
    delete[] par2idx;
    for (int i = 0; i < row; ++i) {
        delete[] cost_mat[i];
    }
    delete[] cost_mat;

    return nnode - sum;
}

void Partition::checkIntegrity() {
    int* par2idx = new int[nnode + 1];
    std::memset(par2idx, -1, sizeof(int) * (nnode + 1));

    int order_cnt = 0;
    for (int i = 0; i < nnode; i++) {
        if (par2idx[pvertex[i]] == -1) {
            par2idx[pvertex[i]] = order_cnt++;
        }
        assert(pvertex[i] >= 0);
    }

    assert(order_cnt == pbkt_size - 1);
    
    delete[] par2idx;
}