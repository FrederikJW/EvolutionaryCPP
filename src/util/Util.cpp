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
