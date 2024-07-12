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

private:
    int nnode;
    int** matrix;

    void allocateMatrix();
    void deallocateMatrix();
};

#endif // GRAPH_H
