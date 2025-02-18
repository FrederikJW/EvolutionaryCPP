#include "CPPInstance.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <limits>
#include <string>
#include <list>


CPPInstance::CPPInstance(const std::string& FileName): edgeSampler(nullptr) {
    Load(FileName);
}

CPPInstance::CPPInstance(int nnode, int** matrix) : edgeSampler(nullptr) {
    LoadFromMatrix(nnode, matrix);
}

CPPInstance::~CPPInstance() {
    delete edgeSampler;
}

int CPPInstance::getNumberOfNodes() const {
    return mNumberOfNodes;
}

int CPPInstance::getNumberOfEdges() const {
    return numberOfEdges;
}

const std::vector<std::vector<int>>& CPPInstance::getWeights() const {
    return mWeights;
}

const std::vector<std::vector<int>>& CPPInstance::getNegativeWeights() const {
    return mNegativeWeights;
}

const std::vector<int>& CPPInstance::getNeighborSize() const {
    return neighborSize;
}

const std::vector<std::vector<int>>& CPPInstance::getNeighbors() const {
    return neighbors;
}

const std::vector<std::array<int, 3>>& CPPInstance::getEdges() const {
    return edges;
}

int CPPInstance::getWeight(int n1, int n2) const {
    return mWeights[n1][n2];
}

void CPPInstance::InitNegativeWeights() {
    mNegativeWeights.resize(mNumberOfNodes, std::vector<int>(mNumberOfNodes));
    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = 0; j < mNumberOfNodes; ++j) {
            mNegativeWeights[i][j] = -mWeights[i][j];
        }
    }
}

void CPPInstance::InitNeighbors() {
    neighborSize.resize(mNumberOfNodes, 0);
    neighbors.resize(mNumberOfNodes, std::vector<int>(mNumberOfNodes));

    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = i + 1; j < mNumberOfNodes; ++j) {
            if (mWeights[i][j] != 0) {
                neighbors[i][neighborSize[i]++] = j;
                neighbors[j][neighborSize[j]++] = i;
            }
        }
    }
}

void CPPInstance::InitEdges() {
    numberOfEdges = 0;
    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = i + 1; j < mNumberOfNodes; ++j) {
            int weight = mWeights[i][j];
            /*
            edges.emplace_back(std::array<int, 3>{i, j, weight});
            numberOfEdges++;
            */
            if (weight != 0) {
                edges.emplace_back(std::array<int, 3>{i, j, weight});
                numberOfEdges++;
            }
        }
    }
}

void CPPInstance::InitEdgeSampler() {
    std::vector<int> edgeWeights(numberOfEdges);

    for (int i = 0; i < numberOfEdges; i++) {
        int weight = edges[i][2];
        if (weight < 0) weight = -weight;
        edgeWeights[i] = weight;
    }

    edgeSampler = new WeightedRandomSampler(edgeWeights);
}

const std::array<int, 3>& CPPInstance::getSampledRandEdge() {
    return edges[edgeSampler->sample()];
}

void CPPInstance::Allocate() {
    mWeights.resize(mNumberOfNodes, std::vector<int>(mNumberOfNodes));
}

void CPPInstance::LoadFromMatrix(int nnode, int** matrix) {
    mNumberOfNodes = nnode;
    
    Allocate();
    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = i; j < mNumberOfNodes; ++j) {
            mWeights[i][j] = matrix[i][j];
            mWeights[j][i] = matrix[j][i];
        }
    }

    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
}

void CPPInstance::Load(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    int cLine = 0;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);
    std::list<int> cWeights;
    int cValue, Current;

    if (file.is_open()) {
        std::getline(file, line);
        mNumberOfNodes = std::stoi(line);
        Allocate();
        while (std::getline(file, line)) {
            std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
            std::sregex_token_iterator end;
            while (iter != end) {
                if (!iter->str().empty()) {
                    cWeights.push_back(std::stoi(*iter));
                }
                ++iter;
            }
        }
        file.close();
    }

    Current = 0;
    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = i; j < mNumberOfNodes; ++j) {
            mWeights[i][j] = -cWeights.front();
            mWeights[j][i] = -cWeights.front();
            cWeights.pop_front();
            ++Current;
        }
    }
    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
}

void CPPInstance::LoadMIP1(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    int cLine = 0;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);

    if (file.is_open()) {
        std::getline(file, line);
        std::getline(file, line);
        std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
        std::sregex_token_iterator end;
        std::vector<std::string> words(iter, end);
        mNumberOfNodes = std::stoi(words[1]);
        Allocate();
        while (std::getline(file, line)) {
            iter = std::sregex_token_iterator(line.begin(), line.end(), regex_pattern, -1);
            words = std::vector<std::string>(iter, end);
            int Node1 = std::stoi(words[0]) - 1;
            int Node2 = std::stoi(words[1]) - 1;
            int Value = std::stoi(words[2]);
            mWeights[Node1][Node2] = Value;
            mWeights[Node2][Node1] = Value;
        }
        file.close();
    }
    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
}

void CPPInstance::LoadMIP_GT(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    int cLine = 0;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);
    int Jobs, Machines;
    std::vector<std::vector<int>> Pair;

    if (file.is_open()) {
        std::getline(file, line);
        std::replace(line.begin(), line.end(), ';', ' ');
        std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
        std::vector<std::string> words(iter, std::sregex_token_iterator());
        Jobs = std::stoi(words[0]);
        Machines = std::stoi(words[1]);
        Pair.resize(Jobs, std::vector<int>(Machines));

        mNumberOfNodes = Jobs + Machines;
        Allocate();

        for (int i = 0; i < Jobs; ++i) {
            std::getline(file, line);
            iter = std::sregex_token_iterator(line.begin(), line.end(), regex_pattern, -1);
            words = std::vector<std::string>(iter, std::sregex_token_iterator());
            for (int j = 0; j < Machines; ++j) {
                Pair[i][j] = std::stoi(words[j]);
            }
        }

        for (int i = 0; i < mNumberOfNodes; ++i) {
            for (int j = 0; j < mNumberOfNodes; ++j) {
                mWeights[i][j] = 0;
            }
        }

        for (int i = 0; i < Jobs; ++i) {
            for (int j = 0; j < Machines; ++j) {
                if (Pair[i][j] == 0) {
                    mWeights[i][j + Jobs] = -1;
                    mWeights[j + Jobs][i] = -1;
                }
                else {
                    mWeights[i][j + Jobs] = 1;
                    mWeights[j + Jobs][i] = 1;
                }
            }
        }
        file.close();
    }
    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
}

double CPPInstance::GetCForModularityMaximization(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);
    double M;
    int cLine = 0;
    double N;
    double Sum = 0;
    int k = 0;

    if (file.is_open()) {
        std::getline(file, line);
        std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
        std::vector<std::string> words(iter, std::sregex_token_iterator());
        N = std::stod(words[0]);
        M = std::stod(words[1]);

        while (std::getline(file, line)) {
            iter = std::sregex_token_iterator(line.begin(), line.end(), regex_pattern, -1);
            words = std::vector<std::string>(iter, std::sregex_token_iterator());
            std::list<std::string> TempWords;
            for (const auto& word : words) {
                if (std::find(TempWords.begin(), TempWords.end(), word) == TempWords.end()) {
                    TempWords.push_back(word);
                }
            }
            k = TempWords.size() - 1;
            Sum += (k * k) / (4 * M * M);
        }
        file.close();
    }
    return Sum;
}

double CPPInstance::LoadMIP_MM(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    int cLine = 0;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);
    double C = 0;

    if (file.is_open()) {
        std::getline(file, line);
        std::replace(line.begin(), line.end(), ';', ' ');
        std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
        std::vector<std::string> words(iter, std::sregex_token_iterator());
        mNumberOfNodes = std::stoi(words[1]);
        Allocate();

        std::getline(file, line);
        std::replace(line.begin(), line.end(), ';', ' ');
        iter = std::sregex_token_iterator(line.begin(), line.end(), regex_pattern, -1);
        words = std::vector<std::string>(iter, std::sregex_token_iterator());
        C = std::stod(words[1]);

        for (int i = 0; i < mNumberOfNodes; ++i) {
            for (int j = 0; j < mNumberOfNodes; ++j) {
                mWeights[i][j] = 0;
            }
        }

        while (std::getline(file, line)) {
            std::replace(line.begin(), line.end(), ';', ' ');
            iter = std::sregex_token_iterator(line.begin(), line.end(), regex_pattern, -1);
            words = std::vector<std::string>(iter, std::sregex_token_iterator());

            int Node1 = std::stoi(words[0]) - 1;
            int Node2 = std::stoi(words[1]) - 1;
            double tempVal = std::stod(words[2]);

            if (tempVal >= 0)
                tempVal = tempVal;

            int cValue = static_cast<int>(tempVal * 1000000);
            mWeights[Node1][Node2] = cValue;
            mWeights[Node2][Node1] = cValue;
        }
        file.close();
    }
    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
    return C;
}

void CPPInstance::LoadMIP_Convert(const std::string& FileName) {
    std::ifstream file(FileName);
    std::string line;
    int cLine = 0;
    std::string pattern = "\\s+";
    std::regex regex_pattern(pattern);
    std::list<int> AllValues;

    if (file.is_open()) {
        while (std::getline(file, line) && line[0] == '%') {
            // Skip comment lines
        }
        mNumberOfNodes = std::stoi(line);
        Allocate();
        while (std::getline(file, line)) {
            if (line[0] == '%') {
                continue;
            }
            std::sregex_token_iterator iter(line.begin(), line.end(), regex_pattern, -1);
            std::vector<std::string> words(iter, std::sregex_token_iterator());
            for (const auto& word : words) {
                AllValues.push_back(std::stoi(word));
            }
        }
        file.close();
    }

    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = 0; j < mNumberOfNodes; ++j) {
            mWeights[i][j] = 0;
        }
    }

    int Current = AllValues.size() - 1;
    for (int i = 0; i < mNumberOfNodes; ++i) {
        for (int j = i + 1; j < mNumberOfNodes; ++j) {
            mWeights[i][j] = AllValues.back();
            mWeights[j][i] = AllValues.back();
            AllValues.pop_back();
            --Current;
        }
    }
    InitNegativeWeights();
    InitNeighbors();
    InitEdges();
    InitEdgeSampler();
}
