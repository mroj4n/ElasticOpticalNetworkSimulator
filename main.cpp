#include "utils/logger.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "networkStructure/GlobalVariables.hpp"
#include "networkStructure/RequestHandler.hpp"


Logger::Logger logger("log.log", true); // Create a logger object with the log file path and printDebug flag
int main(){
    std::string pathName = "POL12/";
    std::string networkFile = "POL12/pol12.net";
    std::string pathsFile = "POL12/pol12.pat";

    logger.debug("Starting program");
    PreDefinedData::fillData(pathName, networkFile, pathsFile, 1);
    auto t = PreDefinedData::networkDescription->getNodeForID(0);

    Request::RequestHandler requestHandler(PreDefinedData::networkDescription, PreDefinedData::possiblePaths, PreDefinedData::demands);
    PreDefinedData::networkDescription->printLinksStatus();
    requestHandler.runSimulation();
    PreDefinedData::networkDescription->printLinksStatus();
    logger.debug("Ending program");
    
}


void logLinkStatus(){
// log state of PreDefinedData::networkDescription->links
    for (auto& link : PreDefinedData::networkDescription->links)
    {
        logger.debug("Link " + std::to_string(link.id) + " [");
        for (auto& channel : link.slotStatus)
        {
            if(channel == 0){
                logger.debug("0");
            }
            else{
                logger.debug("1");
            }
        }
        logger.debug("]");
    }
}