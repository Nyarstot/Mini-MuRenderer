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

    ResourceEntry RenderPass::GetEntryFromReads(const std::wstring& name) const
    {
        auto it = m_reads.find(name);
        if (it != m_reads.end()) {
            auto entry = m_reads.at(name);
            return entry;
        }

        // TODO: return NULL instead of throw error
        throw std::runtime_error("No reads with given name");
    }

    ResourceEntry RenderPass::GetEntryFromWrites(const std::wstring& name) const
    {
        auto it = m_writes.find(name);
        if (it != m_writes.end()) {
            auto entry = m_writes.at(name);
            return entry;
        }

        // TODO: return NULL instead of throw error
        throw std::runtime_error("No reads with given name");
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

    RenderPass& RenderPass::ReadFrom(Span<ResourceEntry> resources)
    {
        for (auto resource : resources) {
            //assert(resource.type == RenderGraphResourceType::Buffer || resource.type == RenderGraphResourceType::Texture);
            std::wstring name = resource.name;
            m_reads.insert({name, resource});
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
                    m_writes.insert({resource->name, *resource});
                }
            }
        }

        return *this;
    }
}