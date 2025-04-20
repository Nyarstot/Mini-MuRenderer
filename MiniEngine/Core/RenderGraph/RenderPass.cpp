#include "pch.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    RenderPass::RenderPass(const std::wstring& name)
        : m_name(name)
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
}