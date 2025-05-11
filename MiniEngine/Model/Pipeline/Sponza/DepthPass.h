#pragma once

#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace Sponza
{
    class DepthPass : public RenderGraph::RenderPass
    {
    protected:
        void InternalExecute(CommandContext& ctx) override;
        void InternalExecuteMultiAdapter(CommandContext& ctx) override;

    public:
        DepthPass();
        ~DepthPass();

        //void Setup(RenderGraph::RenderGraph& renderGraph) override;

    };
}
