#pragma once

#include "RenderGraph/RenderPass.h"
#include "RenderGraph/Base/Span.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "CommandContext.h"


namespace RenderGraph
{
    class LambdaContextRenderPass final : public RenderPass
    {
    public:
        using executeFunction = std::function<void(GraphicsContext&)>;
        LambdaContextRenderPass(
            const std::wstring& name,
            RenderGraph* renderGraph,
            Span<ResourceEntry> readFrom,
            Span<ResourceEntry* const> writeTo,
            executeFunction exec
        );
        ~LambdaContextRenderPass() = default;

        //RenderPass& ReadFrom(Span<ResourceEntry> resources);
        //RenderPass& WriteTo(Span<ResourceEntry* const> resources);

        void Setup();
        void Execute(GraphicsContext& ctx);


    private:
        executeFunction m_executeFunction;

    };
}

using ReadFromSpan = RenderGraph::Span<RenderGraph::ResourceEntry>;
using WriteToSpan = RenderGraph::Span<RenderGraph::ResourceEntry* const>;

