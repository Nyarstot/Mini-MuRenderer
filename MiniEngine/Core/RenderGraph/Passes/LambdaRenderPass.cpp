#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"


namespace RenderGraph
{
    LambdaRenderPass::LambdaRenderPass(const std::wstring& name, executeFunction execFunc)
        : RenderPass(name), m_executeFunction(std::move(execFunc))
    {
    }

    void LambdaRenderPass::InternalExecute(CommandContext& ctx)
    {
        m_executeFunction(ctx.GetCommandList());
    }

    void LambdaRenderPass::InternalExecuteMultiAdapter(CommandContext& ctx)
    {
        std::vector<ComPtr<ID3D12CommandQueue>> commandQueues;
        for (std::size_t i = 0; i < Graphics::g_multiAdapterManager.GetDeviceCount(); i++) {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

            ComPtr<ID3D12CommandQueue> queue;
            Graphics::g_multiAdapterManager.GetDevice(i)->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
            commandQueues.push_back(queue);
        }

        ComPtr<ID3D12CommandAllocator> allocator;
        ASSERT_SUCCEEDED(Graphics::g_multiAdapterManager.GetDevice(this->m_execAdapterIndex)->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)
        ));

        ComPtr<ID3D12GraphicsCommandList> commandList;
        ASSERT_SUCCEEDED(Graphics::g_multiAdapterManager.GetDevice(this->m_execAdapterIndex)->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)
        ));

        // Exec

        UINT64 fenceValue = 1;
        for (std::size_t depAdapter : this->m_dependentAdapters) {
            commandQueues[depAdapter]->Wait(this->m_sharedFence.Get(), fenceValue - 1);
        }

        commandList->Reset(allocator.Get(), nullptr);
        m_executeFunction(ctx.GetCommandList());
        commandList->Close();

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueues[this->m_execAdapterIndex]->ExecuteCommandLists(1, ppCommandLists);
        commandQueues[this->m_execAdapterIndex]->Signal(this->m_sharedFence.Get(), fenceValue);
        fenceValue++;

        for (std::size_t i = 0; i < Graphics::g_multiAdapterManager.GetDeviceCount(); i++) {
            commandQueues[i]->Signal(this->m_sharedFence.Get(), fenceValue);
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