#pragma once

#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderPass.h"


namespace SponzaPasses
{
    class LightShadowsPass final : public RenderGraph::RenderPass
    {
    protected:
        void InternalExecute(CommandContext& ctx) override;
        void InternalExecuteMultiAdapter(CommandContext& ctx) override;

    public:
        LightShadowsPass(RenderGraph::RenderGraph* renderGraph);
        ~LightShadowsPass() = default;
        void foo() {
            bool a = false;
        }
    };
}
