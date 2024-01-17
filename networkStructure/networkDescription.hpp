#pragma once
#include "PreDefinedData.hpp"

namespace PreDefinedData {
class NetworkDescription {
public:
    NetworkDescription(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + filename);
        }

        int numNodes, numLinks;
        file >> numNodes >> numLinks;

        // Create nodes
        for (int i = 0; i < numNodes; ++i) {
            node n;
            n.id = i;
            n.name = "id" + std::to_string(i);
            nodes.push_back(n);
        }

        // Create links
        uint32_t distance;
        int linkID = 0;
        for (int i = 0; i < numNodes; ++i) {
            for (int j = 0; j < numNodes; ++j) {
                file >> distance;
                if (distance != 0) {
                    link l;
                    l.id = linkID++;
                    l.from = nodes[i];
                    l.to = nodes[j];
                    l.distance = distance;
                    links.push_back(l);
                }
            }
        }
    }

    node& getNodeForID(const int64_t& id) {
        for (auto& n : nodes) {
            if (n.id == id) {
                return n;
            }
        }
        logger.error("Node not found");
        throw std::runtime_error("Node not found");
    }

    link& getLinkForID(const int64_t& id) {
        for (auto& l : links) {
            if (l.id == id) {
                return l;
            }
        }
        logger.error("Link not found");
        throw std::runtime_error("Link not found");
    }

    size_t getNumberOfNodes() const {
        return nodes.size();
    }

    size_t getNumberOfLinks() const {
        return links.size();
    }

    void printLinksStatus(){
        for (auto& link : links)
        {
            std::string s = "";
            s += "Link " + std::to_string(link.id) + " [";

            for (auto& channel : link.slotStatus)
            {
                if(channel == 0){
                    s += "0";
                }
                else{
                    s += "1";
                }
            }
            s += "]";

            logger.debug(s);
        }
    }
    std::vector<node> nodes;
    std::vector<link> links;
};

} // namespace PreDefinedData
