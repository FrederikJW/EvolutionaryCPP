#include "Recorder.h"
#include <iostream>
#include <string>
#include <cstring>

Recorder::Recorder(char* graphFilePath, std::string configCode) {
    bestScore = 0;
    clock_t currentTime = clock();

    char path_cwd[PATH_MAX];
    char file_name[PATH_MAX];
    char base_file_path[PATH_MAX];
    char config_code[100];
    std::strcpy(config_code, configCode.c_str());

#ifdef _WIN32
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    _splitpath(graphFilePath, drive, dir, fname, ext);
    sprintf(file_name, "%s/rec/%s_%s_%d.rec", _getcwd(path_cwd, PATH_MAX), fname, config_code, param_seed);
    sprintf(base_file_path, "%s/rec/%s_%s_%d", _getcwd(path_cwd, PATH_MAX), fname, config_code, param_seed);
#else
    char* graph_name = basename(graphFilePath);
    getcwd(path_cwd, PATH_MAX);
    sprintf(file_name, "%s/rec/%s_%s_%d.rec", path_cwd, graph_name, config_code, param_seed);
    sprintf(base_file_path, "%s/rec/%s_%s_%d", path_cwd, graph_name, config_code, param_seed);
#endif
    baseFilePath = base_file_path;
    const std::string filename = file_name;
    file.open(filename, std::ios::out | std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
    }
}

Recorder::~Recorder() {
    if (file.is_open()) {
        file.close();
    }
}

void Recorder::writeLine(const std::string& line) {
    if (file.is_open()) {
        file << line << std::endl;
    }
    else {
        std::cerr << "Error: File is not open." << std::endl;
    }
}

void Recorder::writeLineTo(std::ofstream* fileToWrite, const std::string& line) {
    if (fileToWrite->is_open()) {
        *fileToWrite << line << std::endl;
    }
    else {
        std::cerr << "Error: File is not open." << std::endl;
    }
}

void Recorder::setStartTime(clock_t time) {
    startTime = time;
}

clock_t Recorder::getCurrentTime(clock_t time) {
    return time - startTime;
}

void Recorder::recordSolution(Partition* partition, clock_t time) {
    if (partition->getValue() > bestScore) {
        bestScore = partition->getValue();
        clock_t currentTime = getCurrentTime(time);
        std::string line = std::to_string(bestScore) + ";" + std::to_string(currentTime);
        writeLine(line);
    }
}

void Recorder::enter(const std::string& method) {
    enteredTime[method] = clock();
}

void Recorder::exit(const std::string& method) {
    if (enteredTime.find(method) != enteredTime.end()) {
        // Key exists
        timeRecord[method].push_back(clock() - enteredTime[method]);
        enteredTime.erase(method);
    }
    else {
        // Key does not exist
        std::cout << "Method '" << method << "' was never entered." << std::endl;
    }
}

void Recorder::writeTimeResults() {
    for (const auto& pair : timeRecord) {
        writeLine("Results of " + pair.first);

        int avg = 0;
        int max = 0;
        int min = INT16_MAX;
        for (const auto& time : pair.second) {
            avg += time;
            if (time > max) max = time;
            if (time < min) min = time;
        }
        avg /= pair.second.size();
        writeLine("\tmax: " + to_string(max));
        writeLine("\tmin: " + to_string(min));
        writeLine("\tavg: " + to_string(avg));
    }
}

void Recorder::createTimeResultsFiles() {
    for (const auto& pair : timeRecord) {
        std::ofstream timeFile;
        std::string filename = baseFilePath + "_" + pair.first + ".rec";
        timeFile.open(filename, std::ios::out | std::ios::app);
        
        for (const auto& time : pair.second) {
            writeLineTo(&timeFile, to_string(time));
        }

        timeFile.close();
    }
}
