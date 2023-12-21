#pragma once

#include "PreDefinedData.hpp"
namespace PreDefinedData
{

class Demands
{
public:
    Demands(const std::string pathname, const uint16_t& numberOfDemands, std::shared_ptr<NetworkDescription> networkDescription)
    {
        for (uint16_t i = 0; i < numberOfDemands; i++)
        {
            for (uint16_t j = 0; j < NUMBER_OF_DEMAND_FILES_PER_DEMAND; j++)
            {
                const std::string filename = pathname + "demands_" + std::to_string(i) + "/" + std::to_string(j) + ".txt";
                uint16_t id = i * NUMBER_OF_DEMAND_FILES_PER_DEMAND + j;
                parseDemandFile(filename, networkDescription, id);
            }
        }
    }

    size_t getNumberOfDemands() const
    {
        return demands.size();
    }

    std::vector<Demand>& getDemands()
    {
        return demands;
    }

private:
    std::vector<Demand> demands;
    void parseDemandFile(const std::string& filename, std::shared_ptr<NetworkDescription> networkDescription, uint16_t& id)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file " + filename);
        }
        Demand demand;
        int64_t sourceNode, destinationNode;
        char trafficClass; // ignore
        float bitrate;

        file >> sourceNode >> destinationNode >> trafficClass;
        demand.from = networkDescription->getNodeForID(sourceNode);
        demand.to = networkDescription->getNodeForID(destinationNode);
        demand.id = id;
        while (file >> bitrate)
        {
            demand.bitrates.push_back(bitrate);
        }
        demands.push_back(demand);
    }
};

} // namespace PreDefinedData
