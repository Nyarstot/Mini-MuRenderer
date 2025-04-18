#pragma once

#include <vector>
#include <memory>
#include <string>
#include <queue>
#include <unordered_map>
#include <algorithm>
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

    struct GraphNodeIndexed final
    {
        std::wstring dataIndex;
        std::vector<std::size_t> inputEdges;
        std::vector<std::size_t> outputEdges;
    };

    struct GraphEdgeIndexed
    {
        std::wstring dataIndex;
        std::size_t fromNode;
        std::size_t toNode;
    };

    template <typename NodeType, typename EdgeType>
    class BaseGraphStoraged
    {
        using NodeTypePtr = std::unique_ptr<NodeType>;
        using EdgeTypePtr = std::unique_ptr<EdgeType>;

    protected:
        std::vector<std::size_t> m_executionOrder;
        std::vector<GraphNodeIndexed> m_nodes;
        std::vector<GraphEdgeIndexed> m_edges;
        std::unordered_map<std::wstring, NodeTypePtr> m_nodeStorage;
        std::unordered_map<std::wstring, EdgeTypePtr> m_edgeStorage;

    public:
        BaseGraphStoraged() = default;
        virtual ~BaseGraphStoraged() = default;

        void StoreNode(NodeTypePtr nodeData, const std::wstring& nodeId) {
            m_nodeStorage.emplace(nodeId, std::move(nodeData));
        }

        std::size_t AddNode(const std::wstring& nodeId) {
            assert(m_nodeStorage.find(nodeId) != m_nodeStorage.end());
            std::size_t id = m_nodes.size();
            m_nodes.push_back({ std::move(nodeId), {}, {} });
            return id;
        }

        void StoreEdge(EdgeTypePtr edgeData, const std::wstring& nodeId) {
            m_edgeStorage.emplace(nodeId, std::move(edgeData));
        }

        std::size_t AddEdge(std::size_t fromNode, std::size_t toNode, const std::wstring& nodeId) {
            auto nodesSize = m_nodes.size();
            assert(fromNode < m_nodes.size() && toNode < m_nodes.size());

            assert(m_edgeStorage.find(nodeId) != m_edgeStorage.end());

            std::size_t id = m_edges.size();
            m_edges.push_back({ std::move(nodeId), fromNode, toNode });

            m_nodes[fromNode].outputEdges.push_back(id);
            m_nodes[toNode].inputEdges.push_back(id);

            return id;
        }

        void Clear() {
            for (auto& node : m_nodes) {
                node.inputEdges.clear();
                node.outputEdges.clear();
            }
            m_nodes.clear();
            m_edges.clear();
        }

        const NodeTypePtr& GetNodeData(std::wstring id) const {
            assert(m_nodeStorage.find(id) != m_nodeStorage.end());
            return m_nodeStorage.at(id);
        }

        NodeTypePtr& GetNodeData(std::wstring id) {
            assert(m_nodeStorage.find(id) != m_nodeStorage.end());
            return m_nodeStorage.at(id);
        }

        const GraphNodeIndexed& GetNode(std::size_t id) const {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        GraphNodeIndexed& GetNode(std::size_t id) {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        const EdgeTypePtr& GetEdgeData(std::wstring id) const {
            assert(m_edgeStorage.find(id) != m_edgeStorage.end());
            return m_edgeStorage[id];
        }

        EdgeTypePtr& GetEdgeData(std::wstring id) {
            assert(m_edgeStorage.find(id) != m_edgeStorage.end());
            return m_edgeStorage[id];
        }

        const GraphEdgeIndexed& GetEdge(std::size_t id) const {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        GraphEdgeIndexed& GetEdge(std::size_t id) {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        std::size_t GetNodeCount() const { return m_nodes.size(); }
        std::size_t GetEdgeCount() const { return m_edges.size(); }

    };
}