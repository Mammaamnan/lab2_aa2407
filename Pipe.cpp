#include "Pipe.h"
#include <stdexcept>
#include <vector>

Pipe::Pipe() : id(0), name(""), diameter(0.0), in_repair(false) {}
Pipe::Pipe(uint64_t id_, const std::string& name_, double diameter_, bool in_repair_)
    : id(id_), name(name_), diameter(diameter_), in_repair(in_repair_) {}

uint64_t Pipe::getId() const { return id; }
std::string Pipe::getName() const { return name; }
double Pipe::getDiameter() const { return diameter; }
bool Pipe::isInRepair() const { return in_repair; }

void Pipe::setName(const std::string& n) { name = n; }
void Pipe::setDiameter(double d) { diameter = d; }
void Pipe::setInRepair(bool r) { in_repair = r; }

std::string Pipe::serialize() const {
    std::ostringstream os;
    // id|name|diameter|in_repair
    os << id << '|' << name << '|' << diameter << '|' << (in_repair ? 1 : 0);
    return os.str();
}

Pipe Pipe::deserialize(const std::string& line) {
    // split by '|'
    std::vector<std::string> parts;
    std::string cur;
    for (char c : line) {
        if (c == '|') { parts.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    parts.push_back(cur);
    if (parts.size() != 4) throw std::runtime_error("Pipe::deserialize: wrong format");
    uint64_t id = std::stoull(parts[0]);
    std::string name = parts[1];
    double diameter = std::stod(parts[2]);
    bool in_repair = (parts[3] != "0");
    return Pipe(id, name, diameter, in_repair);
}