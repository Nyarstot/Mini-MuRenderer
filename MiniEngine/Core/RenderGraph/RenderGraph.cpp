#include "pch.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphAllocator.h"
#include "GraphicsCore.h"


namespace RenderGraph
{
    RenderGraph::RenderGraph(const std::wstring& name)
        : BaseGraph<RenderPass, RenderGraphEdgeResourceData>(), m_allocator(65536)
    {
        ASSERT_SUCCEEDED(Graphics::g_Device->CreateFence(
            0, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, IID_PPV_ARGS(&m_sharedFence)
        ));

        m_title = name;
    }

    ResourceEntry& RenderGraph::GetRegisteredResourceEntry(const std::wstring& name)
    {
        return m_resourceRegistry.GetRegisteredResourceEntry(name);
    }

    bool RenderGraph::IsResourceRegistered(const std::wstring& name) const
    {
        return m_resourceRegistry.IsResourceRegistered(name);
    }

    //RenderPass& RenderGraph::AddRenderPass(const std::wstring& name)
    //{
    //    //RenderPass* newRenderPass = m_allocator.Construct<RenderPass>(name);
    //}

    const std::wstring& RenderGraph::GetName() const
    {
        return m_title;
    }

    void RenderGraph::Setup()
    {
    }

    void RenderGraph::Build()
    {
    }

    void RenderGraph::Compile()
    {
        m_executionOrder.clear();
        std::vector<std::size_t> inDegree(GetNodeCount(), 0);
        std::vector<std::size_t> readyNodes;

        for (std::size_t i = 0; i < GetNodeCount(); i++) {
            inDegree[i] = GetNode(i).inputEdges.size();
        }

        for (std::size_t i = 0; i < GetNodeCount(); i++) {
            if (inDegree[i] == 0) readyNodes.push_back(i);
        }

        // Process nodes

        while (!readyNodes.empty()) {
            std::size_t nodeId = readyNodes.back();
            readyNodes.pop_back();
            m_executionOrder.push_back(nodeId);

            // Update in-degree
            const auto& node = GetNode(nodeId);
            for (std::size_t edgeId : node.outputEdges) {
                const auto& edge = GetEdge(edgeId);
                if (--inDegree[edge.toNode] == 0)
                    readyNodes.push_back(edge.toNode);
            }
        }

        if (m_executionOrder.size() != GetNodeCount()) {
            throw std::runtime_error("Render graph contains cycles");
        }
    }

    void RenderGraph::Execute(GraphicsContext& ctx)
    {
        m_fenceValue = 1;

        for (std::size_t nodeId : m_executionOrder) {
            auto& node = GetNode(nodeId);
            node.data->Execute(ctx);
        }

        ctx.Finish();
    }

    void RenderGraph::Clear()
    {
        BaseGraph<RenderPass, RenderGraphEdgeResourceData>::Clear();
    }
}
