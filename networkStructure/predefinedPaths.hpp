#pragma once


#include "PreDefinedData.hpp"

namespace PreDefinedData
{

#define NUMBER_OF_PATHS_PER_PAIR 30



class PossiblePaths
{
public:
    PossiblePaths(const std::string filename, std::shared_ptr<NetworkDescription> networkDescription)
    {
        
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file " + filename);
        }

        std::string line;

        // read first line
        std::getline(file, line);
        std::istringstream linestream(line);
        int numPaths;
        linestream >> numPaths;

        int numOfNodes = (int)networkDescription->getNumberOfNodes();

        for (int i = 0; i < numOfNodes; ++i)
        {
            for (int j = 0; j < numOfNodes; ++j)
            {
                if (i != j)
                {
                    for (int k = 0; k < NUMBER_OF_PATHS_PER_PAIR; ++k)
                    {
                        Path p;
                        p.from = networkDescription->getNodeForID(i);
                        p.to = networkDescription->getNodeForID(j);

                        // Read the line for this path
                        std::getline(file, line);
                        std::istringstream linestream(line);
                        int isLinkUsed;

                        int linkID = 0;

                        while (linestream >> isLinkUsed)
                        {
                            if (isLinkUsed != 0)
                            {
                                p.links.push_back(networkDescription->getLinkForID(linkID));
                            }
                            linkID++;
                        }
                        paths.push_back(p);
                    }
                }
            }
        }
        createPathsMap(paths);
    }

    std::vector<Path> getPathsForNodePair(const node& from, const node& to) const
    {   std::vector<Path> pathsForNodePair;
        for (auto& path : paths)
        {
            if(path.from.id == from.id && path.to.id == to.id)
            {
                pathsForNodePair.push_back(path);
            }
        }
        if (pathsForNodePair.size() == 0)
        {
            throw std::runtime_error("No paths found for node pair");
        }
        return pathsForNodePair;
    }

private:
    std::vector<Path> paths;
    std::map<std::pair<node, node>, std::vector<Path>> pathsMap;

    void createPathsMap(std::vector<Path> paths)
    {
        for (auto& p : paths)
        {
            std::pair<node, node> key = std::make_pair(p.from, p.to);
            pathsMap[key].push_back(p);
        }
    }
};

} // namespace GivenPaths
