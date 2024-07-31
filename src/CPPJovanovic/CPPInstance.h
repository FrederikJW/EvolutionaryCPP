#ifndef CPPINSTANCE_H
#define CPPINSTANCE_H

#include <vector>
#include <string>

class CPPInstance {
private:
    int mNumberOfNodes;
    std::vector<std::vector<int>> mWeights;
    std::vector<std::vector<int>> mNegativeWeights;

public:
    CPPInstance(const std::string& FileName);

    int getNumberOfNodes() const;
    const std::vector<std::vector<int>>& getWeights() const;
    const std::vector<std::vector<int>>& getNegativeWeights() const;
    int getWeight(int n1, int n2) const;

    void InitNegativeWeights();
    void Allocate();
    void Load(const std::string& FileName);
    void LoadMIP1(const std::string& FileName);
    void LoadMIP_GT(const std::string& FileName);
    double GetCForModularityMaximization(const std::string& FileName);
    double LoadMIP_MM(const std::string& FileName);
    void LoadMIP_Convert(const std::string& FileName);
};

#endif // CPPINSTANCE_H
