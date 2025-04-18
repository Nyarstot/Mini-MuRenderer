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

    void LambdaRenderPass::Execute(CommandContext& ctx)
    {
        m_executeFunction(ctx.GetCommandList());
    }

    LambdaContextRenderPass::LambdaContextRenderPass(const std::wstring& name, executeContextFunction execFunc)
        : RenderPass(name), m_executeFunction(std::move(execFunc))
    {
    }

    void LambdaContextRenderPass::Setup(RenderGraph& renderGraph)
    {

    }

    void LambdaContextRenderPass::Execute(CommandContext& ctx)
    {
        m_executeFunction(ctx);
    }
}