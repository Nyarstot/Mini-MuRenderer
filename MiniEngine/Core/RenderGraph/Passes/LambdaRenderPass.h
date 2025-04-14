#pragma once

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
        void Execute(ID3D12GraphicsCommandList* cmdList) override;


    private:
        executeFunction m_executeFunction;

    };
}