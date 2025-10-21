#ifndef COMPRESSORSTATION_H
#define COMPRESSORSTATION_H

#include <string>
#include <cstdint>

class CompressorStation {
private:
    uint64_t id;
    std::string name;
    int total_workshops;      // общее количество цехов
    int working_workshops;    // число задействованных (работающих)
    std::string classification;

public:
    CompressorStation();
    CompressorStation(uint64_t id_, const std::string& name_, int total_, int working_, const std::string& classification_);

    uint64_t getId() const;
    std::string getName() const;
    int getTotalWorkshops() const;
    int getWorkingWorkshops() const;
    std::string getClassification() const;

    void setName(const std::string& n);
    void setTotalWorkshops(int t);
    void setWorkingWorkshops(int w);
    void setClassification(const std::string& c);

    double percentIdle() const; // процент незадействованных цехов
    std::string serialize() const;
    static CompressorStation deserialize(const std::string& line);
};

#endif // COMPRESSORSTATION_H