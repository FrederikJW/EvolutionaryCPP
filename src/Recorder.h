// class for recording benchmark values and writing them to a file or console
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
    Recorder(char* graphFilePath, std::string configCode, bool _makeFile);

    // Destructor that closes the file
    ~Recorder();

    // Method to write a line to the file
    void writeLine(const std::string& line);
    void writeLineTo(std::ofstream* file, const std::string& line);
    void setStartTime(clock_t time);
    clock_t getCurrentTime(clock_t time);
    void recordSolution(Partition* partition, clock_t time);
    void enter(const std::string& method);
    void exit(const std::string& method);
    void writeTimeResults();
    void clearTimeResults();
    void createTimeResultsFiles();
    double clockToSeconds(clock_t time);

private:
    std::string baseFilePath;
    std::ofstream file;
    clock_t startTime;
    int bestScore;
    std::map<std::string, std::vector<clock_t>> timeRecord;
    std::map<std::string, clock_t> enteredTime;
    bool makeFile;
};

#endif // RECORDER_H
