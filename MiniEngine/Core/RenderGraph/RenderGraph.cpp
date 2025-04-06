#include "pch.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    std::shared_ptr<RenderGraphResource> RenderGraph::CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc)
    {
        auto resource = std::make_shared<RenderGraphResource>(name, desc);
        m_resources[name] = resource;
        return resource;
    }

    std::shared_ptr<RenderGraphResource> RenderGraph::GetResource(const std::wstring& name) const
    {
        auto it = m_resources.find(name);
        return it != m_resources.end() ? it->second : nullptr;
    }

    void RenderGraph::Compile()
    {

        /////////////////////
        // Topological sort
        /////////////////////

        m_executionOrder.clear();
        std::vector<std::size_t> inDegree(GetNodeCount(), 0);
        std::vector<std::size_t> readyNodes;

        // Calculate in-degree for each node
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            const auto& node = GetNode(i);
            for (std::size_t edgeId : node.inputEdges) {
                const auto& edge = GetEdge(edgeId);
                inDegree[edge.fromNode]++;
            }
        }

        // Find initial ready nodes (in-degree = 0)
        for (std::size_t i = 0; i < GetNodeCount(); ++i) {
            if (inDegree[i] == 0) {
                readyNodes.push_back(i);
            }
        }

        /////////////////////
        // Process
        /////////////////////

        while (!readyNodes.empty()) {
            std::size_t nodeId = readyNodes.back();
            readyNodes.pop_back();
            m_executionOrder.push_back(nodeId);

            // Update in-degree of neighbors
            const auto& node = GetNode(nodeId);
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                if (--inDegree[edge.toNode] == 0) {
                    readyNodes.push_back(edge.toNode);
                }
            }
        }

        // Check for cycles
        if (m_executionOrder.size() != GetNodeCount()) {
            throw std::runtime_error("Render graph contains cycles");
        }
    }

    void RenderGraph::Execute(ID3D12GraphicsCommandList& commandList)
    {
        for (std::size_t nodeId : m_executionOrder) {
            auto& node = GetNode(nodeId);
            node.data->Execute(commandList);
        }
    }

    D3D12_RESOURCE_STATES RenderGraph::GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const
    {
        return resource->GetDescription().initialState;
    }
}