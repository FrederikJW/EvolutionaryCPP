#ifndef GRAPH_H
#define GRAPH_H

#include <string>

class Graph {
public:
    Graph();
    ~Graph();

    void load(const std::string& filename);

    int getNodeCount() const;
    int** getMatrix();
    int** getNegativeMatrix();
    void setKnownbest(int value);
    int getKnownbest();

private:
    int nnode;
    int** matrix;
    int** negativeMatrix;
    int knownbest;

    void allocateMatrix();
    void deallocateMatrix();
};

#endif // GRAPH_H
