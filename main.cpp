#include "utils/logger.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "networkStructure/GlobalVariables.hpp"
#include "networkStructure/RequestHandler.hpp"

#define NUMBER_OF_DEMANDS 9

Logger::Logger logger("log.log", true); // Create a logger object with the log file path and printDebug flag
Logger::Logger networkStatePrinter("networkState.log", true);
int main() {
    std::string pathName = "POL12/";
    std::string networkFile = "POL12/pol12.net";
    std::string pathsFile = "POL12/pol12.pat";
    for (uint16_t i = 0; i < NUMBER_OF_DEMANDS; i++) {
        logger.changeFilePath("POL12logs/demand" + std::to_string(i) + ".log");
        networkStatePrinter.changeFilePath("POL12logs/networkState_demand" + std::to_string(i) + ".log");
        networkStatePrinter.writeNetworkState("seconds,demand_id,bitrate,isAllocated,numberOfSlots,modulationFormat,numberOfChannels");

        std::cout << "Starting Demand " << i << std::endl;
        PreDefinedData::fillData(pathName, networkFile, pathsFile, i);
        Request::RequestHandler requestHandler(PreDefinedData::networkDescription, PreDefinedData::possiblePaths, PreDefinedData::demands);
        requestHandler.runSimulation();
        PreDefinedData::networkDescription->printLinksStatus();
        std::cout << "Ending Demand " << i << std::endl;
    }

    pathName = "US26/";
    networkFile = "US26/us26.net";
    pathsFile = "US26/us26.pat";
    for (uint16_t i = 0; i < NUMBER_OF_DEMANDS; i++) {
        logger.changeFilePath("US26logs/demand" + std::to_string(i) + ".log");
        networkStatePrinter.changeFilePath("US26logs/networkState_demand" + std::to_string(i) + ".log");
        networkStatePrinter.writeNetworkState("seconds,demand_id,bitrate,isAllocated,numberOfSlots,modulationFormat,numberOfChannels");

        std::cout << "Starting Demand " << i << std::endl;
        PreDefinedData::fillData(pathName, networkFile, pathsFile, i);
        Request::RequestHandler requestHandler(PreDefinedData::networkDescription, PreDefinedData::possiblePaths, PreDefinedData::demands);
        requestHandler.runSimulation();
        PreDefinedData::networkDescription->printLinksStatus();
        std::cout << "Ending Demand " << i << std::endl;
    }
    // logger.debug("Starting program");
    // PreDefinedData::fillData(pathName, networkFile, pathsFile, 1);
    // auto t = PreDefinedData::networkDescription->getNodeForID(0);

    // Request::RequestHandler requestHandler(PreDefinedData::networkDescription, PreDefinedData::possiblePaths, PreDefinedData::demands);
    // PreDefinedData::networkDescription->printLinksStatus();
    // PreDefinedData::possiblePaths->printPaths();
    // requestHandler.runSimulation();
    // PreDefinedData::networkDescription->printLinksStatus();
    // logger.debug("Ending program");

}