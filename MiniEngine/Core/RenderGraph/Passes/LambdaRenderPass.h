#pragma once

#include "CommandContext.h"
#include "RenderGraph/RenderPass.h"


namespace RenderGraph
{
    class LambdaRenderPass : public RenderPass
    {
    public:
        using executeFunction = std::function<void(ID3D12GraphicsCommandList*)>;
        LambdaRenderPass(const std::wstring& name, executeFunction execFunc);
        ~LambdaRenderPass() = default;

        void Setup(RenderGraph& renderGraph) override;
        void Execute(CommandContext& ctx) override;


    private:
        executeFunction m_executeFunction;

    };

    class LambdaContextRenderPass : public RenderPass
    {
    public:
        using executeContextFunction = std::function<void(CommandContext&)>;
        LambdaContextRenderPass(const std::wstring& name, executeContextFunction execFunc);
        ~LambdaContextRenderPass() = default;

        void Setup(RenderGraph& renderGraph) override;
        void Execute(CommandContext& ctx) override;


    private:
        executeContextFunction m_executeFunction;

    };
}