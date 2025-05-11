#include "pch.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    RenderPass::RenderPass(const std::wstring& name, RenderGraph* renderGraph)
        : m_name(name), m_renderGraph(renderGraph)
    {
    }

    void RenderPass::Setup(const GraphicsContext& ctx)
    {

    }

    void RenderPass::Execute(CommandContext& ctx)
    {
        if (!m_multiAdapterAllowed) {
            this->InternalExecute(ctx);
        }
        else
        {
            this->InternalExecuteMultiAdapter(ctx);
        }
    }

    const std::wstring& RenderPass::GetName() const
    {
        return m_name;
    }

    void RenderPass::SetMultiAdapterAllowed(bool allowed, std::size_t adapterIndex)
    {
        m_multiAdapterAllowed = allowed;
        m_execAdapterIndex = adapterIndex;
    }

    bool RenderPass::IsMultiAdapterAllowed() const
    {
        return m_multiAdapterAllowed;
    }

    void RenderPass::AddDependentAdapter(std::size_t adapterIndex)
    {
        m_dependentAdapters.push_back(adapterIndex);
    }

    void RenderPass::SetSharedFence(ComPtr<ID3D12Fence> fence)
    {
        m_sharedFence = fence;
    }

    RenderPass& RenderPass::ReadFrom(Span<const ResourceEntry> resources)
    {
        for (auto resource : resources) {
            if (resource.resource->IsValid()) {
                assert(resource.type == RenderGraphResourceType::Buffer || resource.type == RenderGraphResourceType::Texture);
                m_reads.insert(resource);
            }
        }

        return *this;
    }

    RenderPass& RenderPass::WriteTo(Span<ResourceEntry* const> resources)
    {
        for (auto resource : resources) {
            if (resource && resource->resource->IsValid()) {
                assert(resource->type == RenderGraphResourceType::Buffer || resource->type == RenderGraphResourceType::Texture);

                auto iter = m_reads.find(*resource);
                if (iter == m_reads.end()) {
                    m_writes.insert(*resource);
                }
            }
        }

        return *this;
    }
}