#pragma once
#include <memory>
#include "NetworkDescription.hpp"
#include "PredefinedPaths.hpp"
#include "Demands.hpp"
#include "../utils/logger.hpp"

namespace PreDefinedData
{
extern std::shared_ptr<NetworkDescription> networkDescription;
extern std::shared_ptr<PossiblePaths> possiblePaths;
extern std::shared_ptr<Demands> demands;
extern bool dataFilled;

void fillData(const std::string& pathName, const std::string& networkFile, const std::string& pathsFile, const uint16_t& demandNumber = 0);

}