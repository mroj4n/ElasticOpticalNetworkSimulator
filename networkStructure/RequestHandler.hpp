#pragma once
#include "data.hpp"
#include "PreDefinedData.hpp"
#include<iostream>
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

        //init
        for (auto& demand : demands->getDemands())
        {
            demand.bitrate_counter = 0;

            auto path = selectPath(possiblePaths->getPathsForNodePair(demand.from, demand.to), demand);
            demand.selectedPath = path;
            handleDemand(demand);
            allocateSlot(demand);

            std::cout << "Demand " << demand.id << " is allocated" << " with number of channels " << demand.numberOfChannels << " and modulation format " << demand.assignedModulationFormat.name << " distance " << demand.selectedPath.getHighestDistance() << std::endl;

        }


        for (uint16_t i = 1; i <= 100/*NUMBER_OF_SECONDS_TO_RUN*/; i++)
        {
            for (auto& demand : demands->getDemands())
            {
                demand.bitrate_counter = i;
                auto refDemand = demand;
                auto path = selectPath(possiblePaths->getPathsForNodePair(demand.from, demand.to), demand);
                demand.selectedPath = path;
                handleDemand(demand);
                

                if (!demand.didDemandChange(refDemand))
                {
                    continue;
                }

                allocateSlot(demand);
                if (demand.didDemandChange(refDemand) && demand.isAllocated) {
                    std::cout << "Demand " << demand.id << " is deallocated" << " new bitrate " << demand.bitrates[demand.bitrate_counter] << " new modulation format " << demand.assignedModulationFormat.name << " old bitrate " << refDemand.bitrates[refDemand.bitrate_counter-1] << " old modulation format " << refDemand.assignedModulationFormat.name << std::endl;
                }

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

    void handleDemand(PreDefinedData::Demand& demand)
    {
        uint16_t numberOfChannels = 1;

        auto modulationFormat = givenModulationFormats.getBestModulationFormat(demand.selectedPath.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);
        while (!modulationFormat.has_value() && numberOfChannels < MAX_NUMBER_OF_CHANNELS)
        {
            numberOfChannels++;
            modulationFormat = givenModulationFormats.getBestModulationFormat(demand.selectedPath.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);
        }

        if (modulationFormat == std::nullopt)
        {
            throw std::runtime_error("Could not find a modulation format for the demand");
        }

        if (modulationFormat.value() != demand.assignedModulationFormat)
        {
            demand.assignedModulationFormat = modulationFormat.value();
            demand.numberOfChannels = numberOfChannels;
            return;
        }


    }

    void allocateSlot(PreDefinedData::Demand& demand)
    {
        auto refLink = demand.selectedPath.links;
        for (auto& link : demand.selectedPath.links)
        {
            auto& globalLink = networkDescription->getLinkForID(link.id);
            auto index = globalLink.getIndexForFreeSlots(demand.assignedModulationFormat.numberOfSlots);
            if (!index.has_value())
            {
                //deallocate all previous links
            }

            globalLink.allocate(index.value(), demand.assignedModulationFormat.numberOfSlots);
        }
        demand.isAllocated = true;
    }
private:


    std::shared_ptr<PreDefinedData::NetworkDescription> networkDescription;
    std::shared_ptr<PreDefinedData::PossiblePaths> possiblePaths;
    std::shared_ptr<PreDefinedData::Demands> demands;
    PreDefinedData::ModulationFormats givenModulationFormats;
};
}