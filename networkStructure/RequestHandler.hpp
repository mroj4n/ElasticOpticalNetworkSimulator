#pragma once
#include "data.hpp"
#include "PreDefinedData.hpp"

namespace Request
{
#define NUMBER_OF_SECONDS_TO_RUN 288
#define MAX_NUMBER_OF_CHANNELS 100
struct NetworkStatus
{
    //TODO 
};



class RequestHandler
{
public:
    RequestHandler(std::shared_ptr<PreDefinedData::NetworkDescription> networkDescription, std::shared_ptr<PreDefinedData::PossiblePaths> possiblePaths, std::shared_ptr<PreDefinedData::Demands> demands)
        : networkDescription(networkDescription), possiblePaths(possiblePaths), demands(demands)
    {
    };

    void runSimulation() {
        std::cout << "Running simulation" << std::endl;

        for (uint16_t i = 0; i <= NUMBER_OF_SECONDS_TO_RUN; i++)
        {
            for (const auto& demand : demands->getDemands())
            {
                if (demand.isAllocated)
                    continue;
                

            }
        }

    }

    PreDefinedData::Path selectPath(const std::vector<PreDefinedData::Path>& paths, const PreDefinedData::Demand& demand)
    {
        for (const auto& path : paths)
        {
            if (path.from == demand.from && path.to == demand.to)
                return path;
                //TODO: check if path is not already used
        }
        throw std::runtime_error("Could not find a path for the demand");
    }

    void handleDemand(PreDefinedData::Demand& demand, const PreDefinedData::Path& path)
    {
        uint16_t numberOfChannels = 1;

        auto modulationFormat = givenModulationFormats.getBestModulationFormat(path.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);

        while (modulationFormat != std::nullopt && numberOfChannels < MAX_NUMBER_OF_CHANNELS)
        {
            numberOfChannels++;
            modulationFormat = givenModulationFormats.getBestModulationFormat(path.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);
        }

        if (modulationFormat == std::nullopt)
        {
            throw std::runtime_error("Could not find a modulation format for the demand");
        }

        demand.bitrate_counter++;

        if (modulationFormat.value() != demand.assignedModulationFormat)
        {
            demand.assignedModulationFormat = modulationFormat.value();
            demand.numberOfChannels = numberOfChannels;

            allocateSlot();

            demand.isAllocated = true;
            return;
        }


    }

    void allocateSlot()
    {
    }
private:
    std::shared_ptr<PreDefinedData::NetworkDescription> networkDescription;
    std::shared_ptr<PreDefinedData::PossiblePaths> possiblePaths;
    std::shared_ptr<PreDefinedData::Demands> demands;
    PreDefinedData::ModulationFormats givenModulationFormats;
};
}