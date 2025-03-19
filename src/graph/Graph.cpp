// class to encapsulate graph functionalities and data rewritten from https://github.com/hellozhilu/MDMCP/tree/main/code
#include "Graph.h"
#include <fstream>
#include <iostream>
#include <cstring>

Graph::Graph() : nnode(0), matrix(nullptr), negativeMatrix(nullptr), knownbest(1000000) {}

Graph::~Graph() {
    deallocateMatrix();
}

void Graph::load(const std::string& filename) {
    deallocateMatrix();

    std::ifstream fin(filename);
    if (fin.fail()) {
        std::cerr << "Cannot open file " << filename << std::endl;
        exit(1);
    }
    fin >> nnode;

    allocateMatrix();

    int val;
    for (int i = 0; i < nnode; i++) {
        for (int j = i; j < nnode; j++) {
            if (i == j) {
                matrix[i][j] = matrix[j][i] = 0;
                negativeMatrix[i][j] = negativeMatrix[j][i] = 0;
            }
            else {
                fin >> val;
                matrix[i][j] = matrix[j][i] = val;
                negativeMatrix[i][j] = negativeMatrix[j][i] = -val;
            }
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

int** Graph::getNegativeMatrix() {
    return negativeMatrix;
}

void Graph::setKnownbest(int value) {
    knownbest = value;
}

int Graph::getKnownbest() {
    return knownbest;
}

void Graph::allocateMatrix() {
    matrix = new int* [nnode];
    negativeMatrix = new int* [nnode];
    for (int i = 0; i < nnode; i++) {
        matrix[i] = new int[nnode];
        std::memset(matrix[i], 0, sizeof(int) * nnode);
        negativeMatrix[i] = new int[nnode];
        std::memset(negativeMatrix[i], 0, sizeof(int) * nnode);
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
    if (negativeMatrix != nullptr) {
        for (int i = 0; i < nnode; i++) {
            delete[] negativeMatrix[i];
        }
        delete[] negativeMatrix;
        negativeMatrix = nullptr;
    }
}
