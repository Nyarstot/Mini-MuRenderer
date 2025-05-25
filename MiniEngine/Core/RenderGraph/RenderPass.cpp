#include "pch.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    RenderPass::RenderPass(const std::wstring& name, RenderGraph* parentRenderGraph)
        : m_name(name), m_parentGraph(parentRenderGraph)
    {
    }

    void RenderPass::SetMultiAdapterAllowed(bool allowed, RGCrossAdapterIndex adapter)
    {
        m_crossAdapterInfo.multiGPUAllowed = allowed;
        m_crossAdapterInfo.execAdapterIndex = adapter;
    }

    bool RenderPass::IsMultiAdapterAllowed() const
    {
        return m_crossAdapterInfo.multiGPUAllowed;
    }

    void RenderPass::AddDependentAdapter(RGCrossAdapterIndex adapter)
    {
        m_crossAdapterInfo.dependentAdapters.push_back(adapter);
    }

    void RenderPass::SetSharedFence(ComPtr<ID3D12Fence> sharedFence)
    {
        m_crossAdapterInfo.sharedFence = sharedFence;
    }

    RenderPass& RenderPass::ReadFrom(Span<ResourceEntry> resources)
    {
        for (auto resource : resources) {
            std::wstring name = resource.name;
            m_reads.insert({ name, resource });
        }

        return *this;
    }

    RenderPass& RenderPass::WriteTo(Span<ResourceEntry* const> resources)
    {
        for (auto resource : resources) {
            if (resource) {
                //assert(resource->type == RenderGraphResourceType::Buffer || resource->type == RenderGraphResourceType::Texture);
                auto iter = m_reads.find(resource->name);
                if (iter == m_reads.end()) {
                    m_writes.insert({ resource->name, *resource });
                }
            }
        }

        return *this;
    }

    const std::wstring& RenderPass::GetName() const
    {
        return m_name;
    }
}
