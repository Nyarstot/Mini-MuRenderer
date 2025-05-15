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

        void InitSharedContext();

    protected:
        void InternalExecute(CommandContext& ctx) override;
        void InternalExecuteMultiAdapter(CommandContext& ctx) override;

    private:
        executeFunction m_executeFunction;

        ComPtr<ID3D12GraphicsCommandList> m_sharedCommandList;
        ComPtr<ID3D12CommandAllocator> m_sharedCommandAllocator;
        ComPtr<ID3D12CommandQueue> m_sharedCommandQueues[2];
        bool m_isSharedContextInit = false;

    };

    class LambdaContextRenderPass : public RenderPass
    {
    public:
        using executeContextFunction = std::function<void(CommandContext&)>;
        LambdaContextRenderPass(const std::wstring& name, executeContextFunction execFunc);
        ~LambdaContextRenderPass() = default;

    protected:
        void InternalExecute(CommandContext& ctx) override;
        void InternalExecuteMultiAdapter(CommandContext& ctx) override;

    private:
        executeContextFunction m_executeFunction;

    };
}