#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    //LambdaContextRenderPass::LambdaContextRenderPass(const std::wstring& name, executeFunction exec, RenderGraph* renderGraph)
    //    : RenderPass(name, renderGraph)
    //{
    //}

    LambdaContextRenderPass::LambdaContextRenderPass(const std::wstring& name, RenderGraph* renderGraph, Span<ResourceEntry> readFrom, Span<ResourceEntry* const> writeTo, executeFunction exec)
        : RenderPass(name, renderGraph), m_executeFunction(std::move(exec))
    {
        this->ReadFrom(readFrom);
        this->WriteTo(writeTo);
    }

    void LambdaContextRenderPass::Setup()
    {
    }

    void LambdaContextRenderPass::Execute(GraphicsContext& ctx)
    {
        m_executeFunction(ctx);
    }
}
