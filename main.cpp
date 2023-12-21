#include <iostream>
#include <vector>
#include <string>
#include "networkStructure/data.hpp"
#include "networkStructure/RequestHandler.hpp"

int main(){
    std::string pathName = "POL12/";
    std::string networkFile = "POL12/pol12.net";
    std::string pathsFile = "POL12/pol12.pat";


    PreDefinedData::fillData(pathName, networkFile, pathsFile, 2);
    auto t = PreDefinedData::networkDescription->getNodeForID(0);

    Request::RequestHandler requestHandler(PreDefinedData::networkDescription, PreDefinedData::possiblePaths, PreDefinedData::demands);
    requestHandler.runSimulation();

    
}

void printLinkStatus(){
// print state of PreDefinedData::networkDescription->links
    for (auto& link : PreDefinedData::networkDescription->links)
    {
        std::cout << "Link " << link.id << " [";
        for (auto& channel : link.slotStatus)
        {
            if(channel == 0){
                std::cout << "0";
            }
            else{
                std::cout << "1";
            }
        }
        std::cout << "]" << std::endl;
    }
}