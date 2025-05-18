#include "pch.h"
#include "RenderGraph/Passes/LambdaRenderPass.h"
#include "RenderGraph/RenderGraph.h"

#include "MultiGPU/CopyEngine.h"
#include "MultiGPU/SecondaryDeviceContext.h"

#include "GraphicsCore.h"
#include "CommandListManager.h"


using namespace MultiGPU;
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
        auto& secondaryDeviceContext = Graphics::g_secondaryDeviceContext;
        auto& commandManager = Graphics::g_CommandManager;

        {
            //commandManager.GetGraphicsQueue().IncrementFence
        }

        {
            ID3D12CommandList* ppCopyCommandList[] = { MultiGPU::CopyEngine::GetCopyCommandList().Get() };
            MultiGPU::CopyEngine::GetCopyQueue()->ExecuteCommandLists(_countof(ppCopyCommandList), ppCopyCommandList);
        }

        {
            secondaryDeviceContext.GetDirectCommandList()->Reset(secondaryDeviceContext.GetDirectCommandAllocator(), nullptr);
            m_executeFunction(ctx.GetCommandList());
            secondaryDeviceContext.GetDirectCommandList()->Close();

            ID3D12CommandList* ppCommandLists[] = { secondaryDeviceContext.GetDirectCommandList() };
            secondaryDeviceContext.GetDirectCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        }
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