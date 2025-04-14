#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    LambdaRenderPass::LambdaRenderPass(const std::wstring& name, executeFunction execFunc)
        : RenderPass(name), m_executeFunction(std::move(execFunc))
    {
    }

    void LambdaRenderPass::Setup(RenderGraph& renderGraph)
    {

    }

    void LambdaRenderPass::Execute(ID3D12GraphicsCommandList* cmdList)
    {
        m_executeFunction(cmdList);
    }
}