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
    void setKnownbest(int value);
    int getKnownbest();

private:
    int nnode;
    int** matrix;
    int knownbest;

    void allocateMatrix();
    void deallocateMatrix();
};

#endif // GRAPH_H
