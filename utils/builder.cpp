#include "../networkStructure/GlobalVariables.hpp"
#include <string>

#include <iostream>
#include <exception>

namespace PreDefinedData
{
    std::shared_ptr<NetworkDescription> networkDescription = nullptr;
    std::shared_ptr<PossiblePaths> possiblePaths = nullptr;
    std::shared_ptr<Demands> demands = nullptr;
    bool dataFilled = false;

    void fillData(const std::string &pathName, const std::string &networkFile, const std::string &pathsFile, const uint16_t &demandNumber)
    {
        try
        {
            networkDescription = std::make_shared<NetworkDescription>(networkFile);
            possiblePaths = std::make_shared<PossiblePaths>(pathsFile, networkDescription);
            demands = std::make_shared<Demands>(pathName, demandNumber, networkDescription);

            dataFilled = true;
            logger.info("Data for demand " + std::to_string(demandNumber) + " filled");
        }
        catch (const std::exception &e)
        {
            logger.error("Error while filling data");
            logger.error(e.what());
            std::cerr << e.what() << '\n';
        }
    } // fillData(
} // namespace PreDefinedData