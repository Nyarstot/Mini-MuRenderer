#pragma once

#include <vector>
#include <memory>
#include <string>
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
    protected:
        std::vector<GraphNode<NodeType>> m_nodes;
        std::vector<GraphEdge<EdgeType>> m_edges;
        std::wstring m_title;

    public:
        virtual ~BaseGraph() = default;

        std::size_t AddNode(std::unique_ptr<NodeType> nodeData) {
            std::size_t id = m_nodes.size();
            m_nodes.push_back({ std::move(nodeData), {}, {} });
            return id;
        }

        std::size_t AddEdge(std::size_t fromNode, std::size_t toNode, std::unique_ptr<EdgeType> edgeData) {
            auto nodesSize = m_nodes.size();
            assert(fromNode < m_nodes.size() && toNode < m_nodes.size());

            std::size_t id = m_edges.size();
            m_edges.push_back({ std::move(edgeData), fromNode, toNode });

            m_nodes[fromNode].outputEdges.push_back(id);
            m_nodes[toNode].inputEdges.push_back(id);

            return id;
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