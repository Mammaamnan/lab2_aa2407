#ifndef MANAGER_H
#define MANAGER_H

#include "Pipe.h"
#include "CompressorStation.h"
#include <vector>
#include <string>

class Manager {
private:
    std::vector<Pipe> pipes;
    std::vector<CompressorStation> stations;
    uint64_t next_id;
    std::string log_filename;

    void logAction(const std::string& msg);

public:
    Manager();
    Manager(const std::string& logFile);

    // public logging wrapper (was private logAction)
    void writeLog(const std::string& msg);

    uint64_t makeId();

    // pipes operations
    uint64_t addPipe(const std::string& name, double diameter, bool in_repair);
    bool removePipeById(uint64_t id);
    Pipe* findPipeById(uint64_t id);
    std::vector<Pipe*> findPipesByName(const std::string& substring);
    std::vector<Pipe*> findPipesByRepairFlag(bool in_repair);
    const std::vector<Pipe>& getPipes() const;

    // compressor stations operations
    uint64_t addStation(const std::string& name, int total, int working, const std::string& classification);
    bool removeStationById(uint64_t id);
    CompressorStation* findStationById(uint64_t id);
    std::vector<CompressorStation*> findStationsByName(const std::string& substring);
    std::vector<CompressorStation*> findStationsByIdlePercent(double minIdlePercent);
    const std::vector<CompressorStation>& getStations() const;

    // save/load
    bool saveToFile(const std::string& filename);
    bool loadFromFile(const std::string& filename);

    // batch edit pipes by IDs (setter functions)
    void batchEditPipes(const std::vector<uint64_t>& ids, const std::string& newName, double newDiameter, int changeRepairFlag); // changeRepairFlag: -1 - no change, 0 - set false, 1 - set true

    // logging filename change
    void setLogFilename(const std::string& filename);
};

#endif // MANAGER_H