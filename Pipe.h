#ifndef PIPE_H
#define PIPE_H

#include <string>
#include <cstdint>
#include <sstream>

class Pipe {
private:
    uint64_t id;
    std::string name;
    double diameter; // mm or chosen unit
    bool in_repair;

public:
    Pipe();
    Pipe(uint64_t id_, const std::string& name_, double diameter_, bool in_repair_);

    // getters / setters
    uint64_t getId() const;
    std::string getName() const;
    double getDiameter() const;
    bool isInRepair() const;

    void setName(const std::string& n);
    void setDiameter(double d);
    void setInRepair(bool r);

    // serialization to single line (safe, '|' as separator)
    std::string serialize() const;
    static Pipe deserialize(const std::string& line);
};

#endif // PIPE_H