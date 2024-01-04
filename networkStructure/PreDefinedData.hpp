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
namespace PreDefinedData
{

#define NUMBER_OF_SLOTS_PER_NODE 320
#define NUMBER_OF_DEMAND_FILES_PER_DEMAND 99 // 99 demand files per demand: Given dataset

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
        for (int i = index; i < index + numberOfSlots; ++i)
        {
            slotStatus[i] = true;
        }
    }

    // method to free slots in the current link based on the index from checkSlots
    void deallocate(const int& index, const int& numberOfSlots)
    {
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

    bool operator!=(const Path& other) const
    {
        return id != other.id;
    }

    bool doesAllLinksHaveFreeSlotsForChannels(uint16_t numberOfChannels) const
    {
        for (const auto& link : links)
        {
            if (!link.getIndexForFreeSlots(numberOfChannels).has_value())
            {
                return false;
            }
        }
        return true;
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
    uint16_t numberOfChannels = 0;
    uint16_t bitrate_counter = 0;
    Path selectedPath;

    ModulationFormats::ModulationFormat assignedModulationFormat;

    bool operator==(const Demand& other) const
    {
        return id == other.id;
    }

    bool didDemandChange(const Demand& other, bool printChanges = false) const
    {
        if (other.from != from || other.to != to)
        {
            throw std::runtime_error("Demands are not the same");
        }
        if (printChanges) {
            //print what is actually different
            if (isAllocated != other.isAllocated)
            {
                std::cout << "Demand " << id << " isAllocated changed from " << other.isAllocated << " to " << isAllocated << std::endl;
            }
            if (numberOfChannels != other.numberOfChannels)
            {
                std::cout << "Demand " << id << " numberOfChannels changed from " << other.numberOfChannels << " to " << numberOfChannels << std::endl;
            }
            if (assignedModulationFormat != other.assignedModulationFormat)
            {
                std::cout << "Demand " << id << " assignedModulationFormat changed from " << other.assignedModulationFormat.name << " to " << assignedModulationFormat.name << std::endl;
            }
            if (selectedPath != other.selectedPath)
            {
                std::cout << "Demand " << id << " selectedPath changed from " << other.selectedPath.id << " to " << selectedPath.id << std::endl;
            }
        }

        return isAllocated != other.isAllocated || numberOfChannels != other.numberOfChannels || assignedModulationFormat != other.assignedModulationFormat || selectedPath != other.selectedPath;
    }
};
}