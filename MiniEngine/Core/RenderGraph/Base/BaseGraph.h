#pragma once

#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <assert.h>


namespace RenderGraph
{
    template<typename NodeType>
    struct GraphNode final
    {
        std::unique_ptr<NodeType> data;
        std::vector<std::size_t> inputEdges;
        std::vector<std::size_t> outputEdges;
    };

    template<typename EdgeType>
    struct GraphEdge
    {
        std::unique_ptr<EdgeType> data;
        std::size_t fromNode;
        std::size_t toNode;
    };

    template <typename NodeType, typename EdgeType>
    class BaseGraph
    {
        using NodeTypePtr = std::unique_ptr<NodeType>;
        using EdgeTypePtr = std::unique_ptr<EdgeType>;

    protected:
        std::vector<std::size_t> m_executionOrder;
        std::vector<GraphNode<NodeType>> m_nodes;
        std::vector<GraphEdge<EdgeType>> m_edges;
        std::wstring m_title;

    protected:
        std::vector<std::size_t> TopologicalSort() const {
            std::vector<std::size_t> result;
            result.reserve(m_nodes.size());

            std::vector<std::size_t> inDegree(m_nodes.size(), 0);
            for (const auto& node : m_nodes) {
                inDegree[&node - m_nodes[0]] = node.inputEdges.size();
            }

            std::queue<std::size_t> zeroInDegreeQueue;
            for (std::size_t i = 0; i < inDegree.size(); ++i)
            {
                if (inDegree[i] == 0)
                {
                    zeroInDegreeQueue.push(i);
                }
            }

            // Process nodes
            std::size_t visitedCount = 0;
            while (!zeroInDegreeQueue.empty())
            {
                std::size_t currentNode = zeroInDegreeQueue.front();
                zeroInDegreeQueue.pop();
                result.push_back(currentNode);
                ++visitedCount;

                // Reduce in-degree of all adjacent nodes
                for (const auto& edgeId : m_nodes[currentNode].outputEdges)
                {
                    const auto& edge = m_edges[edgeId];
                    std::size_t adjacentNode = edge.toNode;
                    if (--inDegree[adjacentNode] == 0)
                    {
                        zeroInDegreeQueue.push(adjacentNode);
                    }
                }
            }

            // Check for cycles
            if (visitedCount != m_nodes.size())
            {
                throw std::runtime_error("Graph contains at least one cycle");
            }

            return result;
        }

    public:
        BaseGraph() = default;
        virtual ~BaseGraph() = default;

        std::size_t AddNode(NodeTypePtr nodeData) {
            std::size_t id = m_nodes.size();
            m_nodes.push_back({ std::move(nodeData), {}, {} });
            return id;
        }

        std::size_t AddEdge(std::size_t fromNode, std::size_t toNode, EdgeTypePtr edgeData) {
            auto nodesSize = m_nodes.size();
            assert(fromNode < m_nodes.size() && toNode < m_nodes.size());

            std::size_t id = m_edges.size();
            m_edges.push_back({ std::move(edgeData), fromNode, toNode });

            m_nodes[fromNode].outputEdges.push_back(id);
            m_nodes[toNode].inputEdges.push_back(id);

            return id;
        }

        void Clear() {
            for (auto& node : m_nodes) {
                if (node.data) {
                    node.data.reset();
                }

                node.inputEdges.clear();
                node.outputEdges.clear();
            }
            m_nodes.clear();

            for (auto& edge : m_edges) {
                if (edge.data) {
                    edge.data.reset();
                }
            }
            m_edges.clear();
        }

        const GraphNode<NodeType>& GetNode(std::size_t id) const {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        GraphNode<NodeType>& GetNode(std::size_t id) {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        const GraphEdge<EdgeType>& GetEdge(std::size_t id) const {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        GraphEdge<EdgeType>& GetEdge(std::size_t id) {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        std::size_t GetNodeCount() const { return m_nodes.size(); }
        std::size_t GetEdgeCount() const { return m_edges.size(); }

    };

}