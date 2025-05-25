#pragma once

#include "RenderGraph/RenderPass.h"
#include "CommandContext.h"


namespace RenderGraph
{
    class LambdaContextRenderPass final : public RenderPass
    {
    public:
        using executeFunction = std::function<void(CommandContext&)>;
        LambdaContextRenderPass(const std::wstring& name, executeFunction exec, RenderGraph* renderGraph);
        ~LambdaContextRenderPass() = default;

        void Setup();
        void Execute(CommandContext& ctx);


    private:
        executeFunction m_executeFunction;

    };
}
