#pragma once
#include "GlobalVariables.hpp"
#include "PreDefinedData.hpp"
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
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
        logger.info("Running simulation");

        for (auto& demand : demands->getDemands())
        {
            demand.bitrate_counter = 0;

            handleDemand(demand);
            auto path = selectPath(possiblePaths->getPathsForNodePair(demand.from, demand.to), demand);
            demand.selectedPath = path.value();
            allocateSlot(demand);

            logger.info("Demand " + std::to_string(demand.id) + " is allocated" + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " distance " + std::to_string(demand.selectedPath.getHighestDistance()));

        }


        for (uint16_t i = 1; i < NUMBER_OF_SECONDS_TO_RUN; i++)
        {
            logger.info("Running simulation for " + std::to_string(i) + " seconds");
            for (auto& demand : demands->getDemands())
            {
                logger.debug("Running simulation for demand " + std::to_string(demand.id) + " for " + std::to_string(i) + " seconds");
                auto refDemand = demand;
                demand.bitrate_counter = i;
                // Break simulation if there is no demand at current counter
                if (demand.bitrates.size() <= demand.bitrate_counter)
                {
                    logger.error("Demand " + std::to_string(demand.id) + " has no bitrate at counter " + std::to_string(demand.bitrate_counter));
                    std::cout << "Fatal error " << "Demand " << std::to_string(demand.id) << " has no bitrate at counter " << std::to_string(demand.bitrate_counter)<<std::endl;
                    std::cout<<"ENDING SIMULATION"<<std::endl;
                    break;
                }

                handleDemand(demand);
                auto path = selectPath(possiblePaths->getPathsForNodePair(demand.from, demand.to), demand);
                if(!path.has_value())
                {
                    demand.isAllocated = false;
                    logger.error("Could not find a path for demand " + std::to_string(demand.id));
                    continue;
                }

                demand.selectedPath = path.value();


                if (!demand.didDemandChange(refDemand))
                {
                    logger.info("Demand " + std::to_string(demand.id) + " did not change from " + demand.assignedModulationFormat.name + " bitrate changed from " + std::to_string(refDemand.bitrates[refDemand.bitrate_counter]) + " to " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " iterator:"+std::to_string(i));
                    continue;
                }
                logger.info("Demand " + std::to_string(demand.id) + " changed from " + refDemand.assignedModulationFormat.name + " to " + demand.assignedModulationFormat.name + " bitrate changed from " + std::to_string(refDemand.bitrates[refDemand.bitrate_counter]) + " to " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " iterator:"+std::to_string(i));
                allocateSlot(demand);


            }

        }

        std::cout << "Simulation finished" << std::endl;
        // print all demands and their allocated slots if they are allocated
        logger.info("Simulation finished");
        logger.info("Printing all demands and their allocated slots");
        for (const auto& demand : demands->getDemands())
        {
            if (demand.isAllocated)
            {
                std::cout << "Demand " << demand.id << " is allocated" << " with number of channels " << demand.numberOfChannels << " and modulation format " << demand.assignedModulationFormat.name << " distance " << demand.selectedPath.getHighestDistance() << std::endl;
                logger.info("Demand " + std::to_string(demand.id) + " is allocated" + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " distance " + std::to_string(demand.selectedPath.getHighestDistance()));
            }
            else
            {
                std::cout << "Demand " << demand.id << " is not allocated" << std::endl;
                logger.info("Demand " + std::to_string(demand.id) + " is not allocated");
            }
        }
        networkDescription->printLinksStatus();

    }

    std::optional<PreDefinedData::Path> selectPath(const std::vector<PreDefinedData::Path>& possiblePaths, const PreDefinedData::Demand& demand)
    {
        for (const auto& path : possiblePaths)
        {
            if (path.from == demand.from && path.to == demand.to)
            {
                bool pathFree = true;
                for(const auto& link : path.links)
                {
                    auto& globalLink = networkDescription->getLinkForID(link.id);
                    if (!globalLink.getIndexForFreeSlots(demand.assignedModulationFormat.numberOfSlots).has_value())
                    {
                        logger.warning("Path " + std::to_string(path.id) + " is not selected for demand " + std::to_string(demand.id) + " because it is not free");
                        logger.warning("amount of space needed: " + std::to_string(demand.assignedModulationFormat.numberOfSlots) );//+ " amount of space available: " + std::to_string(globalLink.getNumberOfFreeSlots()));
                        // print link ids for path
                        std::string linkList = "LinkIds in path" + std::to_string(path.id)+": ";
                        for (const auto& link : path.links)
                        {
                            linkList+=std::to_string(link.id) + " ";
                        }
                        logger.warning(linkList);
                        networkDescription->printLinksStatus();
                        pathFree = false;
                        break;
                    }
                }
                if (pathFree)
                {
                    logger.debug("Path " + std::to_string(path.id) + " is selected for demand " + std::to_string(demand.id));
                    return path;
                }

            }
        }
        logger.error("Could not find a path for the demand " + std::to_string(demand.id));
        return std::nullopt;
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
            logger.error("Could not find a modulation format for the demand " + std::to_string(demand.id));
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
        if (demand.isAllocated)
        {
            for (uint16_t i = 0; i < demand.allocatedPath.links.size(); i++)
            {
                auto& globalLink = networkDescription->getLinkForID(demand.allocatedPath.links[i].id);
                globalLink.deallocate(demand.allocatedPath.slotIndex[i], demand.assignedModulationFormat.numberOfSlots);
            }
        }


        auto refLink = demand.selectedPath.links;
        demand.allocatedPath.links.clear();
        demand.allocatedPath.slotIndex.clear();
        demand.isAllocated = false;
        for (auto& link : demand.selectedPath.links)
        {
            auto& globalLink = networkDescription->getLinkForID(link.id);
            auto index = globalLink.getIndexForFreeSlots(demand.assignedModulationFormat.numberOfSlots); //check is redundant as we already checked in selectPath
            if (!index.has_value())
            {
                logger.error("Could not allocate slots for demand " + std::to_string(demand.id));
                        
                for (uint16_t i = 0; i < demand.allocatedPath.links.size(); i++)
                {
                    auto& globalLink = networkDescription->getLinkForID(demand.allocatedPath.links[i].id);
                    globalLink.deallocate(demand.allocatedPath.slotIndex[i], demand.assignedModulationFormat.numberOfSlots);
                }

                break;
            }

            globalLink.allocate(index.value(), demand.assignedModulationFormat.numberOfSlots);
            demand.allocatedPath.links.push_back(link);
            demand.allocatedPath.slotIndex.push_back(index.value());
        }
        if (demand.allocatedPath.links.size() != demand.selectedPath.links.size())
        {
            std::cout << "Could not allocate slots for demand " << demand.id << std::endl;
            logger.error("Could not allocate slots for demand " + std::to_string(demand.id));
            demand.isAllocated = false;
            return;
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