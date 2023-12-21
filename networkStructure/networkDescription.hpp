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

    node getNodeForID(const int64_t& id) const {
        for (const auto& n : nodes) {
            if (n.id == id) {
                return n;
            }
        }
        throw std::runtime_error("Node not found");
    }

    link getLinkForID(const int64_t& id) const {
        for (const auto& l : links) {
            if (l.id == id) {
                return l;
            }
        }
        throw std::runtime_error("Link not found");
    }

    size_t getNumberOfNodes() const {
        return nodes.size();
    }

    size_t getNumberOfLinks() const {
        return links.size();
    }


    std::vector<node> nodes;
    std::vector<link> links;
};

} // namespace PreDefinedData
