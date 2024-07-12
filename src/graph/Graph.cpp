#include "Graph.h"
#include <fstream>
#include <iostream>
#include <cstring>

Graph::Graph() : nnode(0), matrix(nullptr) {}

Graph::~Graph() {
    deallocateMatrix();
}

void Graph::load(const std::string& filename) {
    std::ifstream fin(filename);
    if (fin.fail()) {
        std::cerr << "Cannot open file " << filename << std::endl;
        exit(1);
    }
    fin >> nnode;

    deallocateMatrix();
    allocateMatrix();

    int val;
    for (int i = 0; i < nnode; i++) {
        for (int j = i; j < nnode; j++) {
            fin >> val;
            matrix[i][j] = matrix[j][i] = -val;
        }
    }
    fin.close();
}

int Graph::getNodeCount() const {
    return nnode;
}

int** Graph::getMatrix() {
    return matrix;
}

void Graph::allocateMatrix() {
    matrix = new int* [nnode];
    for (int i = 0; i < nnode; i++) {
        matrix[i] = new int[nnode];
        std::memset(matrix[i], 0, sizeof(int) * nnode);
    }
}

void Graph::deallocateMatrix() {
    if (matrix != nullptr) {
        for (int i = 0; i < nnode; i++) {
            delete[] matrix[i];
        }
        delete[] matrix;
        matrix = nullptr;
    }
}
