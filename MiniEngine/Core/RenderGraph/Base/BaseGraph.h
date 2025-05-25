#pragma once


namespace RenderGraph
{
    template<typename NodeDataType>
    struct GraphNode final
    {
        std::unique_ptr<NodeDataType> data;
        std::vector<std::size_t> inputEdges;
        std::vector<std::size_t> outputEdges;
    };

    template<typename EdgeDataType>
    struct GraphEdge final
    {
        std::unique_ptr<EdgeDataType> data;
        std::size_t fromNode;
        std::size_t toNode;
    };

    template<typename NodeDataType, typename EdgeDataType>
    class BaseGraph
    {
        using NodeDataTypePtr = std::unique_ptr<NodeDataType>;
        using EdgeDataTypePtr = std::unique_ptr<EdgeDataType>;

    protected:
        std::vector<std::size_t> m_executionOrder;
        std::vector<GraphNode<NodeDataType>> m_nodes;
        std::vector<GraphEdge<EdgeDataType>> m_edges;

    public:
        BaseGraph() = default;
        virtual ~BaseGraph() = default;

        std::size_t AddNode(NodeDataTypePtr NodeData) {
            std::size_t id = m_nodes.size();
            m_nodes.push_back({ std::move(NodeData), {}, {} });
            return id;
        }

        std::size_t AddEdge(std::size_t FromNode, std::size_t ToNode, EdgeDataTypePtr EdgeData) {
            auto nodeSize = m_nodes.size();
            ASSERT(FromNode < m_nodes.size() && ToNode < m_nodes.size());

            std::size_t id = m_edges.size();
            m_edges.push_back({ std::move(EdgeData), FromNode, ToNode });

            m_nodes[FromNode].outputEdges.push_back(id);
            m_nodes[ToNode].inputEdges.push_back(id);

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

        const GraphNode<NodeDataType>& GetNode(std::size_t id) const {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        GraphNode<NodeDataType>& GetNode(std::size_t id) {
            assert(id < m_nodes.size());
            return m_nodes[id];
        }

        const GraphEdge<EdgeDataType>& GetEdge(std::size_t id) const {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        GraphEdge<EdgeDataType>& GetEdge(std::size_t id) {
            assert(id < m_edges.size());
            return m_edges[id];
        }

        std::size_t GetNodeCount() const { return m_nodes.size(); }
        std::size_t GetEdgeCount() const { return m_edges.size(); }
    };
}
