#pragma once
#include "GlobalVariables.hpp"
#include "PreDefinedData.hpp"
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
namespace Request
{
// #define NUMBER_OF_SECONDS_TO_RUN 288
#define NUMBER_OF_SECONDS_TO_RUN 288
#define MAX_NUMBER_OF_CHANNELS 100

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
        logger.info("Running simulation for " + std::to_string(0) + " seconds");
        for (auto& demand : demands->getDemands())
        {
            logger.info("Handling demand " + std::to_string(demand.id) + " Bitrate:" + std::to_string(demand.bitrates[demand.bitrate_counter]) + " for " + std::to_string(0) + " seconds");

        
            demand.bitrate_counter = 0;

            //select the first path iwithout checking if its free
            for (const auto& path : possiblePaths->getPathsForNodePair(demand.from, demand.to))
            {
                demand.selectedPath = path;
                break;
            }
        }

        
        for (uint16_t i = 1; i < NUMBER_OF_SECONDS_TO_RUN; i++)
        {
            logger.info("Running simulation for " + std::to_string(i) + " seconds");
            for (auto& demand : demands->getDemands())
            {
                logger.info("Handling demand " + std::to_string(demand.id) + " Bitrate:" + std::to_string(demand.bitrates[demand.bitrate_counter]) + " for " + std::to_string(i) + " seconds");
                std::string networkStateMessage = std::to_string(i) + "," + std::to_string(demand.id) + "," + std::to_string(demand.isAllocated) + "," + std::to_string(demand.assignedModulationFormat.numberOfSlots) + "," + demand.assignedModulationFormat.name +","+ std::to_string(demand.numberOfChannels);
                networkStatePrinter.writeNetworkState(networkStateMessage);

                auto refDemand = demand;
                // Break simulation if there is no demand at current counter
                if (demand.bitrates.size() <= demand.bitrate_counter)
                {
                    logger.error("Demand " + std::to_string(demand.id) + " has no bitrate at counter " + std::to_string(demand.bitrate_counter));
                    std::cout << "Fatal error " << "Demand " << std::to_string(demand.id) << " has no bitrate at counter " << std::to_string(demand.bitrate_counter) << std::endl;
                    std::cout << "Skipping demand" << std::endl;
                    continue;;
                }

                auto path = selectPath(possiblePaths->getPathsForNodePair(demand.from, demand.to), demand);
                if (!path.has_value())
                {
                    demand.changeAllocation(false);
                    logger.error("Could not find a path for demand " + std::to_string(demand.id));
                    continue;
                }
                demand.selectedPath = path.value();
                selectModulationFormat(demand);


                if (!demand.didDemandChange(refDemand) && demand.isAllocated)
                {
                    logger.info("Demand " + std::to_string(demand.id) + " did not change from " + demand.assignedModulationFormat.name + " bitrate changed from " + std::to_string(refDemand.bitrates[refDemand.bitrate_counter - 1]) + " to " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " iterator:" + std::to_string(i));

                    //increase bitrate_counter so we can check the next bitrate
                    demand.bitrate_counter = demand.bitrate_counter + 1;
                    continue;
                }

                logger.info("Demand " + std::to_string(demand.id) + " changed from " + refDemand.assignedModulationFormat.name + " to " + demand.assignedModulationFormat.name + " bitrate changed from " + std::to_string(refDemand.bitrates[refDemand.bitrate_counter]) + " to " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " iterator:" + std::to_string(i));
                deallocate(refDemand, demand);
                allocateSlot(demand);
                

            }

        }

        std::cout << "Simulation finished" << std::endl;
        // print all demands and their allocated slots if they are allocated
        logger.info("Simulation finished");
        logger.info("Printing all demands and their allocated slots");
        // transceiver_count = number_of_allocated_demands * number_of_channel
        uint16_t transceiver_count = 0;
        for (const auto& demand : demands->getDemands())
        {
            if (demand.isAllocated)
            {
                transceiver_count += (demand.numberOfChannels * 2);
                logger.info("Demand " + std::to_string(demand.id) + " is allocated" + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " distance " + std::to_string(demand.selectedPath.getHighestDistance()));
            }
            else
            {
                logger.info("Demand " + std::to_string(demand.id) + " is not allocated");
            }
        }
        std::cout << "transceiver_count " << transceiver_count << std::endl;
        logger.info("Amount of transceivers needed " + std::to_string(transceiver_count) + " with " + std::to_string(demands->getNumberOfDemands()) + " demands");
        networkDescription->printLinksStatus();
    }

    std::optional<PreDefinedData::Path> selectPath(const std::vector<PreDefinedData::Path>& possiblePaths, const PreDefinedData::Demand& demand)
    {
        //select path assumes the paths are sorted in ascending order, so the first path is the shortest path
        for (const auto& path : possiblePaths)
        {
            if (path.from == demand.from && path.to == demand.to)
            {
                bool pathFree = true;
                if (demand.selectedPath == path && demand.isAllocated)
                {
                    return path;
                }
                for (const auto& link : path.links)
                {
                    auto& globalLink = networkDescription->getLinkForID(link.id);

                    if (!globalLink.getIndexForFreeSlots(demand.assignedModulationFormat.numberOfSlots).has_value())
                    {
                        // print link ids for path
                        std::string linkList = "LinkIds in path" + std::to_string(path.id) + ": ";
                        for (const auto& link : path.links)
                        {
                            linkList += std::to_string(link.id) + " ";
                        }
                        linkList += " are not free";
                        logger.warning(linkList);

                        pathFree = false;
                        break;
                    }
                }
                if (pathFree)
                {
                    logger.debug("Path " + std::to_string(path.id) + " is selected for demand " + std::to_string(demand.id) + " with modulation format " + demand.assignedModulationFormat.name + " with number of channels " + std::to_string(demand.numberOfChannels));
                    return path;
                }

            }
        }
        logger.error("Could not find a path for the demand " + std::to_string(demand.id));
        logger.error("Demand is from " + std::to_string(demand.from.id) + " to " + std::to_string(demand.to.id) + " with bitrate " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " is allocated" + std::to_string(demand.isAllocated));

        return std::nullopt;
    }

    void selectModulationFormat(PreDefinedData::Demand& demand)
    {
        uint16_t numberOfChannels = 1;

        auto modulationFormat = givenModulationFormats.getBestModulationFormat(demand.selectedPath.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);
        while (!modulationFormat.has_value() && numberOfChannels < MAX_NUMBER_OF_CHANNELS)
        {
            numberOfChannels++;
            modulationFormat = givenModulationFormats.getBestModulationFormat(demand.selectedPath.getHighestDistance(), demand.bitrates[demand.bitrate_counter], numberOfChannels);
        }

        if (!modulationFormat.has_value())
        {
            logger.error("Could not find a modulation format for the demand " + std::to_string(demand.id));
            throw std::runtime_error("Could not find a modulation format for the demand");
        }

        demand.numberOfChannels = numberOfChannels;
        if (modulationFormat.value() != demand.assignedModulationFormat)
        {
            //deallocate old slots
            if (demand.isAllocated)
            {
                deallocate(demand);
            }
            demand.assignedModulationFormat = modulationFormat.value();


            logger.debug("Demand " + std::to_string(demand.id) + " changed modulation format to " + demand.assignedModulationFormat.name + " with number of channels " + std::to_string(demand.numberOfChannels));
        }
    }

    void allocateSlot(PreDefinedData::Demand& demand)
    {
        auto refLink = demand.selectedPath.links;
        demand.allocatedPath.links.clear();
        demand.allocatedPath.slotIndex.clear();
        demand.changeAllocation(false);
        logger.info("Demand " + std::to_string(demand.id) + " is  being allocated with bitrate " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " ,LongestLinkDistance:" + std::to_string(demand.selectedPath.getHighestDistance()) + " ,pathId:" + std::to_string(demand.selectedPath.id) + ",path:" + demand.selectedPath.getLinksInPath());
        for (auto& link : demand.selectedPath.links)
        {
            auto& globalLink = networkDescription->getLinkForID(link.id);
            auto index = globalLink.getIndexForFreeSlots(demand.assignedModulationFormat.numberOfSlots);
            if (!index.has_value())
            {
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
            logger.warning("Demand " + std::to_string(demand.id) + " is not allocated with bitrate " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " ,LongestLinkDistance:" + std::to_string(demand.selectedPath.getHighestDistance()) + " ,pathId:" + std::to_string(demand.selectedPath.id) + ",path:" + demand.selectedPath.getLinksInPath());

            return;
        }
        demand.changeAllocation(true);
        logger.info("Demand " + std::to_string(demand.id) + " is allocated with bitrate " + std::to_string(demand.bitrates[demand.bitrate_counter]) + " with number of channels " + std::to_string(demand.numberOfChannels) + " and modulation format " + demand.assignedModulationFormat.name + " ,LongestLinkDistance:" + std::to_string(demand.selectedPath.getHighestDistance()) + " ,pathId:" + std::to_string(demand.selectedPath.id) + ",path:" + demand.selectedPath.getLinksInPath());
        demand.bitrate_counter = demand.bitrate_counter + 1;
    }

    void deallocate(PreDefinedData::Demand& refDemand, PreDefinedData::Demand& demand)
    {
        if (!refDemand.isAllocated)
        {
            logger.error("Demand " + std::to_string(refDemand.id) + " is not allocated");
            return;
        }
        for (uint16_t i = 0; i < refDemand.allocatedPath.links.size(); i++)
        {
            auto& globalLink = networkDescription->getLinkForID(refDemand.allocatedPath.links[i].id);
            globalLink.deallocate(refDemand.allocatedPath.slotIndex[i], refDemand.assignedModulationFormat.numberOfSlots);
        }
        demand.changeAllocation(false);
        logger.info("Demand " + std::to_string(refDemand.id) + " is deallocated");
    }
    void deallocate(PreDefinedData::Demand& demand)
    {
        if (!demand.isAllocated)
        {
            logger.error("Demand " + std::to_string(demand.id) + " is not allocated");
            return;
        }
        for (uint16_t i = 0; i < demand.allocatedPath.links.size(); i++)
        {
            auto& globalLink = networkDescription->getLinkForID(demand.allocatedPath.links[i].id);
            globalLink.deallocate(demand.allocatedPath.slotIndex[i], demand.assignedModulationFormat.numberOfSlots);
            logger.info("Deallocated " + std::to_string(demand.assignedModulationFormat.numberOfSlots) + " slots in link " + std::to_string(demand.allocatedPath.links[i].id) + " starting at index " + std::to_string(demand.allocatedPath.slotIndex[i]));
        }
        demand.changeAllocation(false);
        logger.info("Demand " + std::to_string(demand.id) + " is deallocated");
    }

private:

    std::shared_ptr<PreDefinedData::NetworkDescription> networkDescription;
    std::shared_ptr<PreDefinedData::PossiblePaths> possiblePaths;
    std::shared_ptr<PreDefinedData::Demands> demands;
    PreDefinedData::ModulationFormats givenModulationFormats;
};
}