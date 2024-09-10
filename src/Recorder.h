#ifndef RECORDER_H
#define RECORDER_H

#include "Defines.h"
#include "partition/Partition.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <ctime>
#include <string>

class Recorder {
public:
    // Constructor that opens the file
    Recorder(char* graphFilePath, std::string configCode);

    // Destructor that closes the file
    ~Recorder();

    // Method to write a line to the file
    void writeLine(const std::string& line);
    void setStartTime(clock_t time);
    clock_t getCurrentTime(clock_t time);
    void recordSolution(Partition* partition, clock_t time);
    void enter(const std::string& method);
    void exit(const std::string& method);
    void writeTimeResults();

private:
    std::ofstream file;
    clock_t startTime;
    int bestScore;
    std::map<std::string, std::vector<clock_t>> timeRecord;
    std::map<std::string, clock_t> enteredTime;
};

#endif // RECORDER_H
