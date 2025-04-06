#include "pch.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    RenderPass::RenderPass(const std::wstring& name)
        : m_name(name)
    {

    }

    const std::wstring& RenderPass::GetName() const
    {
        return m_name;
    }
}