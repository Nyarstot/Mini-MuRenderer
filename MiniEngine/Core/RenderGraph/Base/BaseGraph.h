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
    /**
    * @brief basic node type of the DAG graph.
    * @tparam NodeType data type hold by the graph node.
    */
    template<typename NodeType>
    struct GraphNode final
    {
        std::unique_ptr<NodeType> data;
        std::vector<std::size_t> inputEdges;
        std::vector<std::size_t> outputEdges;
    };

    /**
    * @brief basic edge type of the DAG graph.
    * @tparam EdgeType data type stored on the edges of the graph.
    */
    template<typename EdgeType>
    struct GraphEdge final
    {
        std::unique_ptr<EdgeType> data;
        std::size_t fromNode;
        std::size_t toNode;
    };

    /**
    * @class BaseGraph BaseGraph.h "Core/RenderGraph/Base/BaseGraph.h"
    * @brief base class for mathematical directional graphs.
    * @tparam NodeType type of the data stored in nodes.
    * @tparam EdgeType type if the data stored on edges.
    */
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

        /**
        * @brief creates and appends new graph node.
        * @param[in] nodeData data to store in the created node.
        * @returns id of the created node.
        */
        std::size_t AddNode(NodeTypePtr nodeData) {
            std::size_t id = m_nodes.size();
            m_nodes.push_back({ std::move(nodeData), {}, {} });
            return id;
        }

        /**
        * @brief creates new edge between two nodes.
        * @param[in] fromNode node from which the connection comes.
        * @param[in] toNode the node to which the connection is received.
        * @param[in] edgeData data to store on the edge.
        */
        std::size_t AddEdge(std::size_t fromNode, std::size_t toNode, EdgeTypePtr edgeData) {
            auto nodesSize = m_nodes.size();
            assert(fromNode < m_nodes.size() && toNode < m_nodes.size());

            std::size_t id = m_edges.size();
            m_edges.push_back({ std::move(edgeData), fromNode, toNode });

            m_nodes[fromNode].outputEdges.push_back(id);
            m_nodes[toNode].inputEdges.push_back(id);

            return id;
        }

        /**
        * @brief clears all the graph
        */
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

        /**
        * @brief gets a specific graph node from a graph by id.
        * @param[in] id integer identifier of the node.
        * @returns constant node from the graph.
        */
        const GraphNode<NodeType>& GetNode(std::size_t id) const {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        /**
        * @brief gets a specific graph node from a graph by id.
        * @param[in] id integer identifier of the node.
        * @returns node from the graph.
        */
        GraphNode<NodeType>& GetNode(std::size_t id) {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        /**
        * @brief gets a specific graph edge from a graph by id.
        * @param[in] id integer identifier of the edge.
        * @returns constant edge from the graph.
        */
        const GraphEdge<EdgeType>& GetEdge(std::size_t id) const {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        /**
        * @brief gets a specific graph edge from a graph by id.
        * @param[in] id integer identifier of the edge.
        * @returns edge from the graph.
        */
        GraphEdge<EdgeType>& GetEdge(std::size_t id) {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        /**
        * @brief gets total node count in the graph.
        * @returns count of nodes in graph.
        */
        std::size_t GetNodeCount() const { return m_nodes.size(); }

        /**
        * @brief gets total edge count in the graph.
        * @returns count of edges in graph.
        */
        std::size_t GetEdgeCount() const { return m_edges.size(); }

        void SetTitle(const std::wstring& name) {
            m_title = name;
        }

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