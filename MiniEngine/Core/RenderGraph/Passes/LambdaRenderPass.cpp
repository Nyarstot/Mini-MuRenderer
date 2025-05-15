#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"

#include "MultiGPU/CopyEngine.h"

#include "GraphicsCore.h"
#include "CommandListManager.h"


namespace RenderGraph
{
    LambdaRenderPass::LambdaRenderPass(const std::wstring& name, executeFunction execFunc)
        : RenderPass(name), m_executeFunction(std::move(execFunc))
    {

    }

    void LambdaRenderPass::InitSharedContext()
    {
        if (!m_isSharedContextInit) {
            for (std::size_t i = 0; i < Graphics::g_multiAdapterManager.GetDeviceCount(); i++) {
                D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                Graphics::g_multiAdapterManager.GetDevice(i)->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_sharedCommandQueues[i]));
            }

            ASSERT_SUCCEEDED(Graphics::g_multiAdapterManager.GetDevice(this->m_execAdapterIndex)->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sharedCommandAllocator)
            ));

            ASSERT_SUCCEEDED(Graphics::g_multiAdapterManager.GetDevice(this->m_execAdapterIndex)->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sharedCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_sharedCommandList)
            ));

            m_isSharedContextInit = true;
        }

    }

    void LambdaRenderPass::InternalExecute(CommandContext& ctx)
    {
        m_executeFunction(ctx.GetCommandList());
    }

    void LambdaRenderPass::InternalExecuteMultiAdapter(CommandContext& ctx)
    {
        UINT64 fenceValue = 1;
        for (std::size_t depAdapter : this->m_dependentAdapters) {
            m_sharedCommandQueues[depAdapter]->Wait(this->m_sharedFence.Get(), fenceValue - 1);
        }

        {
            ID3D12CommandList* ppCopyCommandList[] = { MultiGPU::CopyEngine::GetCopyCommandList().Get() };
            MultiGPU::CopyEngine::GetCopyQueue()->ExecuteCommandLists(_countof(ppCopyCommandList), ppCopyCommandList);
        }

        m_sharedCommandList->Reset(m_sharedCommandAllocator.Get(), nullptr);
        m_executeFunction(ctx.GetCommandList());
        m_sharedCommandList->Close();

        ID3D12CommandList* ppCommandLists[] = { m_sharedCommandList.Get() };
        m_sharedCommandQueues[this->m_execAdapterIndex]->ExecuteCommandLists(1, ppCommandLists);
        m_sharedCommandQueues[this->m_execAdapterIndex]->Signal(this->m_sharedFence.Get(), fenceValue);
        fenceValue++;

        for (std::size_t i = 0; i < Graphics::g_multiAdapterManager.GetDeviceCount(); i++) {
            m_sharedCommandQueues[i]->Signal(this->m_sharedFence.Get(), fenceValue);
            this->m_sharedFence->SetEventOnCompletion(fenceValue, nullptr);
        }
        fenceValue++;
    }

    LambdaContextRenderPass::LambdaContextRenderPass(const std::wstring& name, executeContextFunction execFunc)
        : RenderPass(name), m_executeFunction(std::move(execFunc))
    {
    }

    void LambdaContextRenderPass::InternalExecute(CommandContext& ctx)
    {
        m_executeFunction(ctx);
    }

    void LambdaContextRenderPass::InternalExecuteMultiAdapter(CommandContext& ctx)
    {
    }
}