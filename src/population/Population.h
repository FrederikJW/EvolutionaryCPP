#ifndef POPULATION_H
#define POPULATION_H

#include "../partition/Partition.h"
#include "../Defines.h"
#include <vector>

class Population {
public:
    Population(int poolSize);
    ~Population();

    void updatePopulation();
    int addPopulation(const Partition& partition, int objval);
    int insertPopulationWhenFull(const Partition& partition, int objval);

    Partition& getPartition(int id);
    std::vector<Partition*>& getPartitions() { return partitions; }
    double getAverageObjective() const;
    int getMaxObjective() const;
    int getMinObjective() const;
    int getMinDistance() const;
    int partitionCount();
    int getPoolSize();
    float getAvgDistance();

private:
    std::vector<Partition*> partitions;
    std::vector<int> objValues;
    std::vector<int> distances;

    int obj_max;
    int obj_min;
    int dis_min;
    double obj_ave;
    int poolSize;

    void disposePopulation();
};

#endif // POPULATION_H
