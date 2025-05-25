#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    LambdaContextRenderPass::LambdaContextRenderPass(const std::wstring& name, executeFunction exec, RenderGraph* renderGraph)
        : RenderPass(name, renderGraph)
    {
    }

    void LambdaContextRenderPass::Setup()
    {
    }

    void LambdaContextRenderPass::Execute(CommandContext& ctx)
    {
        m_executeFunction(ctx);
    }
}
