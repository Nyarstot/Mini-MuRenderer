#include "pch.h"
#include "LightShadowsPass.h"
#include "LightManager.h"
#include "EngineProfiling.h"


namespace SponzaPasses
{
    void LightShadowsPass::InternalExecute(CommandContext& ctx)
    {
        using namespace Lighting;
        ScopedTimer _prof(L"LightShadowPass", ctx);

        static uint32_t LightIndex = 0;
        if (LightIndex >= MaxLights)
            return;

        for (auto resource : m_reads) {
            //resource.
        }
        //m_LightShadowTempBuffer
    }

    void LightShadowsPass::InternalExecuteMultiAdapter(CommandContext& ctx)
    {
    }

    LightShadowsPass::LightShadowsPass(RenderGraph::RenderGraph* renderGraph)
        : RenderPass(L"LightShadowPass", renderGraph)
    {
    }
}
