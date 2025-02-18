#include "Util.h"

void verifySolution(Partition* partition, Graph* graph) {
    int score = partition->calculateScore(graph->getMatrix());

    if (partition->getValue() == score) {
        printf("The score of %d has been successfully verified\n", score);
    }
    else {
        printf("The given score of %d does not equal the verification score of %d\n", partition->getValue(), score);
    }
}

double FastExp(double x)
{
    union {
        long long int i;
        double d;
    } tmp;

    tmp.i = static_cast<long long int>(1512775 * x + 1072632447);
    tmp.i <<= 32;
    return tmp.d;
}