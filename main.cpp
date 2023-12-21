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
}