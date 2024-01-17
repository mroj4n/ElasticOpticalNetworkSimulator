#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <optional>
#include <map>
#include "../utils/logger.hpp"
namespace PreDefinedData
{

#define NUMBER_OF_SLOTS_PER_NODE 320
#define NUMBER_OF_DEMAND_FILES_PER_DEMAND 500 // 500 demand files per demand: Given dataset

struct ModulationFormats
{
    struct ModulationFormat
    {
        std::string name;
        uint16_t maximumDistance;
        uint16_t maximumBitrate;
        uint16_t numberOfSlots;

        bool operator!=(const ModulationFormat& other) const
        {
            return name != other.name;
        }
    };
    std::vector<ModulationFormat> modulationFormats = {
            {"QPSK", std::numeric_limits<uint16_t>::max(), 200, 6},
            {"8-QAM", std::numeric_limits<uint16_t>::max(), 400, 9},
            {"16-QAM", 800, 400, 6},
            {"16-QAM-long", 1600, 600, 9},
            {"32-QAM", 200, 800, 9}
    };
    //least amount of slots
    std::optional<ModulationFormat> getBestModulationFormat(const uint32_t& distance, const float& bitrate, const uint16_t& numberOfChannels) const
    {
        float newBitrate = bitrate / numberOfChannels;
        ModulationFormat bestModulationFormat;
        bool foundModulationFormat = false;

        for (const auto& modulationFormat : modulationFormats)
        {
            if (modulationFormat.maximumDistance >= distance && modulationFormat.maximumBitrate >= newBitrate)
            {
                if (!foundModulationFormat)
                {
                    bestModulationFormat = modulationFormat;
                    foundModulationFormat = true;
                }
                else if (modulationFormat.numberOfSlots < bestModulationFormat.numberOfSlots)
                {
                    bestModulationFormat = modulationFormat;
                }
            }
        }
        if (!foundModulationFormat)
        {
            return std::nullopt;
        }
        logger.debug("Best modulation format for distance " + std::to_string(distance) + " and bitrate " + std::to_string(bitrate) + " number of channels " + std::to_string(numberOfChannels) + " is " + bestModulationFormat.name);
        bestModulationFormat.numberOfSlots *= numberOfChannels;
        return bestModulationFormat;
    }
};


struct node
{
    int64_t id;
    std::string name; // optional, so it can be an empty string

    bool operator<(const node& other) const
    {
        return id < other.id;
    }

    bool operator==(const node& other) const
    {
        return id == other.id;
    }

    bool operator!=(const node& other) const
    {
        return id != other.id;
    }
};

struct link
{
    uint16_t id;
    node from;
    node to;
    uint32_t distance; // in kilometers


    bool slotStatus[NUMBER_OF_SLOTS_PER_NODE] = { false }; // false = free, true = occupied

    bool operator<(const link& other) const
    {
        return distance < other.distance;
    }

    bool operator==(const link& other) const
    {
        return id == other.id && from == other.from && to == other.to && distance == other.distance;
    }
    std::optional<uint16_t> getIndexForFreeSlots(uint16_t numberOfSlots) const
    {
        uint16_t counter = 0;
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS_PER_NODE; ++i)
        {
            if (slotStatus[i] == false)
            {
                counter++;
                if (counter == numberOfSlots)
                {
                    return i - counter + 1;
                }
            }
            else
            {
                counter = 0;
            }
        }
        return std::nullopt;
    }

    // method to reserve slots in the current link based on the index from checkSlots
    void allocate(const int& index, const int& numberOfSlots)
    {
        logger.debug("Allocating " + std::to_string(numberOfSlots) + " slots in link " + std::to_string(id) + " starting at index " + std::to_string(index));
        for (int i = index; i < index + numberOfSlots; ++i)
        {
            slotStatus[i] = true;
        }
    }

    // method to free slots in the current link based on the index from checkSlots
    void deallocate(const int& index, const int& numberOfSlots)
    {
        logger.debug("Deallocating " + std::to_string(numberOfSlots) + " slots in link " + std::to_string(id) + " starting at index " + std::to_string(index));
        for (int i = index; i < index + numberOfSlots; ++i)
        {
            slotStatus[i] = false;
        }
    }

};

struct Path
{
    uint16_t id;
    node from;
    node to;
    std::vector<link> links;
    std::vector<int> slotIndex;
    uint32_t getHighestDistance() const
    {
        uint32_t highestDistance = 0;
        for (const auto& link : links)
        {
            if (link.distance > highestDistance)
            {
                highestDistance = link.distance;
            }
        }
        return highestDistance;
    }
    std::string getLinksInPath() const
    {
        std::string s = "[";
        for (const auto& link : links)
        {
            s += std::to_string(link.id) + ",";
        }
        s += "]";
        return s;
    }
    bool operator==(const Path& other) const
    {
        return id == other.id;
    }
    bool operator!=(const Path& other) const
    {
        return id != other.id;
    }
};

struct Demand
{
    // Loaded from file
    uint16_t id;//id is i * NUMBER_OF_DEMAND_FILES_PER_DEMAND + j if the file is from demands_i/j.txt
    node from;
    node to;
    std::vector<float> bitrates;

    bool isAllocated = false;
    struct AllocatedPath
    {
        std::vector<link> links;
        std::vector<int> slotIndex;
    };
    AllocatedPath allocatedPath;
    uint16_t numberOfChannels = 0;
    uint16_t bitrate_counter = 0;
    Path selectedPath;

    ModulationFormats::ModulationFormat assignedModulationFormat= {"QPSK", std::numeric_limits<uint16_t>::max(), 200, 6};

    void changeAllocation(bool isAllocated){
        logger.debug("Demand " + std::to_string(id) + " isAllocated changed from " + std::to_string(this->isAllocated) + " to " + std::to_string(isAllocated));

        if(this->isAllocated == isAllocated)
        {
            logger.error("Demand " + std::to_string(id) + " isAllocated not changed from! ");
        }
        this->isAllocated = isAllocated;
    }
    bool operator==(const Demand& other) const
    {
        return id == other.id;
    }

    bool didDemandChange(const Demand& other) const
    {
        if (other.from != from || other.to != to && other.id != id)
        {
            logger.error("Demands " + std::to_string(id) + " and " + std::to_string(other.id) + " are not the same");
            throw std::runtime_error("Demands are not the same");
        }

        //print what is actually different
        if (isAllocated != other.isAllocated)
        {
            logger.info("Demand " + std::to_string(id) + " isAllocated changed from " + std::to_string(other.isAllocated) + " to " + std::to_string(isAllocated));
        }
        if (numberOfChannels != other.numberOfChannels)
        {
            logger.info("Demand " + std::to_string(id) + " numberOfChannels changed from " + std::to_string(other.numberOfChannels) + " to " + std::to_string(numberOfChannels));
        }
        if (assignedModulationFormat != other.assignedModulationFormat)
        {
            logger.info("Demand " + std::to_string(id) + " assignedModulationFormat changed from " + other.assignedModulationFormat.name + " to " + assignedModulationFormat.name);
        }
        if (selectedPath != other.selectedPath)
        {
            logger.info("Demand " + std::to_string(id) + " selectedPath changed from " + std::to_string(other.selectedPath.id) + " to " + std::to_string(selectedPath.id));
        }


        return isAllocated != other.isAllocated || numberOfChannels != other.numberOfChannels || assignedModulationFormat != other.assignedModulationFormat || selectedPath != other.selectedPath;
    }
};
}