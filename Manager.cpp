#include "Manager.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

Manager::Manager() : next_id(1), log_filename("actions.log") {}
Manager::Manager(const std::string& logFile) : next_id(1), log_filename(logFile) {}

void Manager::setLogFilename(const std::string& filename) {
    log_filename = filename;
    logAction("Log filename changed to: " + filename);
}

// public wrapper to allow logging from outside
void Manager::writeLog(const std::string& msg) {
    logAction(msg);
}

uint64_t Manager::makeId() {
    uint64_t id = next_id++;
    return id;
}

void Manager::logAction(const std::string& msg) {
    std::ofstream os(log_filename, std::ios::app);
    if (!os) return;
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " | " << msg << "\n";
    os.close();
}

// === Pipes
uint64_t Manager::addPipe(const std::string& name, double diameter, bool in_repair) {
    uint64_t id = makeId();
    pipes.emplace_back(id, name, diameter, in_repair);
    logAction("Added pipe id=" + std::to_string(id) + " name=\"" + name + "\" diameter=" + std::to_string(diameter) + " in_repair=" + (in_repair ? "1":"0"));
    return id;
}

bool Manager::removePipeById(uint64_t id) {
    auto it = std::find_if(pipes.begin(), pipes.end(), [id](const Pipe& p){ return p.getId() == id; });
    if (it == pipes.end()) return false;
    logAction("Removed pipe id=" + std::to_string(it->getId()) + " name=\"" + it->getName() + "\"");
    pipes.erase(it);
    return true;
}

Pipe* Manager::findPipeById(uint64_t id) {
    for (auto &p : pipes) if (p.getId() == id) return &p;
    return nullptr;
}

std::vector<Pipe*> Manager::findPipesByName(const std::string& substring) {
    std::vector<Pipe*> res;
    for (auto &p : pipes) {
        if (p.getName().find(substring) != std::string::npos) res.push_back(&p);
    }
    logAction("Searched pipes by name=\"" + substring + "\" -> " + std::to_string(res.size()) + " found");
    return res;
}

std::vector<Pipe*> Manager::findPipesByRepairFlag(bool in_repair) {
    std::vector<Pipe*> res;
    for (auto &p : pipes) {
        if (p.isInRepair() == in_repair) res.push_back(&p);
    }
    logAction("Searched pipes by in_repair=" + std::string(in_repair ? "1":"0") + " -> " + std::to_string(res.size()) + " found");
    return res;
}

const std::vector<Pipe>& Manager::getPipes() const { return pipes; }

// === Stations
uint64_t Manager::addStation(const std::string& name, int total, int working, const std::string& classification) {
    uint64_t id = makeId();
    stations.emplace_back(id, name, total, working, classification);
    logAction("Added station id=" + std::to_string(id) + " name=\"" + name + "\" total=" + std::to_string(total) + " working=" + std::to_string(working));
    return id;
}

bool Manager::removeStationById(uint64_t id) {
    auto it = std::find_if(stations.begin(), stations.end(), [id](const CompressorStation& s){ return s.getId() == id; });
    if (it == stations.end()) return false;
    logAction("Removed station id=" + std::to_string(it->getId()) + " name=\"" + it->getName() + "\"");
    stations.erase(it);
    return true;
}

CompressorStation* Manager::findStationById(uint64_t id) {
    for (auto &s : stations) if (s.getId() == id) return &s;
    return nullptr;
}

std::vector<CompressorStation*> Manager::findStationsByName(const std::string& substring) {
    std::vector<CompressorStation*> res;
    for (auto &s : stations) {
        if (s.getName().find(substring) != std::string::npos) res.push_back(&s);
    }
    logAction("Searched stations by name=\"" + substring + "\" -> " + std::to_string(res.size()) + " found");
    return res;
}

std::vector<CompressorStation*> Manager::findStationsByIdlePercent(double minIdlePercent) {
    std::vector<CompressorStation*> res;
    for (auto &s : stations) { 
        if (s.percentIdle() >= minIdlePercent) res.push_back(&s);
    }
    std::ostringstream oss;
    oss << "Searched stations by minIdlePercent=" << minIdlePercent << " -> " << res.size() << " found";
    logAction(oss.str());
    return res;
}

const std::vector<CompressorStation>& Manager::getStations() const { return stations; }

// === save / load
bool Manager::saveToFile(const std::string& filename) {
    std::ofstream os(filename);
    if (!os) {
        logAction("Failed to save to file: " + filename);
        return false;
    }
    // header: next_id
    os << "NEXT_ID|" << next_id << "\n";
    os << "#PIPES\n";
    for (const auto &p : pipes) os << p.serialize() << "\n";
    os << "#STATIONS\n";
    for (const auto &s : stations) os << s.serialize() << "\n";
    os.close();
    logAction("Saved to file: " + filename + " pipes=" + std::to_string(pipes.size()) + " stations=" + std::to_string(stations.size()));
    return true;
}

bool Manager::loadFromFile(const std::string& filename) {
    std::ifstream is(filename);
    if (!is) {
        logAction("Failed to load from file: " + filename);
        return false;
    }
    pipes.clear();
    stations.clear();
    std::string line;
    enum Section { NONE, PIPES, STATIONS } section = NONE;
    uint64_t loaded_next_id = 1;
    while (std::getline(is, line)) {
        if (line.size() == 0) continue;
        if (line.rfind("NEXT_ID|",0) == 0) {
            try { loaded_next_id = std::stoull(line.substr(8)); }
            catch(...) { loaded_next_id = 1; }
            continue;
        }
        if (line == "#PIPES") { section = PIPES; continue; }
        if (line == "#STATIONS") { section = STATIONS; continue; }
        try {
            if (section == PIPES) {
                Pipe p = Pipe::deserialize(line);
                pipes.push_back(p);
            } else if (section == STATIONS) {
                CompressorStation s = CompressorStation::deserialize(line);
                stations.push_back(s);
            } else {
                // unknown lines ignored
            }
        } catch (const std::exception &e) {
            // ignore malformed line but log
            logAction(std::string("Warning: failed to parse line during load: ") + e.what() + " line=[" + line + "]");
        }
    }
    is.close();
    // ensure next_id is greater than any id found
    uint64_t maxid = 0;
    for (const auto &p : pipes) if (p.getId() > maxid) maxid = p.getId();
    for (const auto &s : stations) if (s.getId() > maxid) maxid = s.getId();
    next_id = std::max(loaded_next_id, maxid + 1);
    logAction("Loaded from file: " + filename + " pipes=" + std::to_string(pipes.size()) + " stations=" + std::to_string(stations.size()) + " next_id=" + std::to_string(next_id));
    return true;
}

// === batch edit pipes
void Manager::batchEditPipes(const std::vector<uint64_t>& ids, const std::string& newName, double newDiameter, int changeRepairFlag) {
    std::ostringstream oss;
    oss << "Batch edit pipes count=" << ids.size() << " newName=\"" << newName << "\" newDiameter=" << newDiameter << " changeRepair=" << changeRepairFlag;
    logAction(oss.str());
    for (uint64_t id : ids) {
        Pipe* p = findPipeById(id);
        if (!p) {
            logAction("Batch edit: cannot find pipe id=" + std::to_string(id));
            continue;
        }
        if (!newName.empty()) p->setName(newName);
        if (newDiameter > 0.0) p->setDiameter(newDiameter);
        if (changeRepairFlag == 0) p->setInRepair(false);
        if (changeRepairFlag == 1) p->setInRepair(true);
        logAction("Batch edited pipe id=" + std::to_string(id));
    }
}