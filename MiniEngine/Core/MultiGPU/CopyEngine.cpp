#include "pch.h"
#include "MultiGPU/CopyEngine.h"
#include "GraphicsCore.h"


namespace MultiGPU
{
    ComPtr<ID3D12CommandAllocator> CopyEngine::s_copyAllocator = nullptr;
    ComPtr<ID3D12CommandQueue> CopyEngine::s_copyCommandQueue = nullptr;
    ComPtr<ID3D12GraphicsCommandList> CopyEngine::s_copyCommandList = nullptr;

    void CopyEngine::Initialize()
    {
        ID3D12Device* primaryDevice = Graphics::g_Device;
        ID3D12Device* secondaryDevice = Graphics::g_SecondaryDevice;

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

        ASSERT_SUCCEEDED(primaryDevice->CreateCommandQueue(
            &queueDesc, IID_PPV_ARGS(&s_copyCommandQueue)
        ));

        ASSERT_SUCCEEDED(primaryDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_COPY,
            IID_PPV_ARGS(&s_copyAllocator)
        ));

        ASSERT_SUCCEEDED(primaryDevice->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_COPY, s_copyAllocator.Get(),
            nullptr, IID_PPV_ARGS(&s_copyCommandList)
        ));

        ASSERT_SUCCEEDED(s_copyCommandList->Close());

    }

    void CopyEngine::CopyResource(GpuResource& dest, GpuResource& src)
    {
        ASSERT_SUCCEEDED(s_copyAllocator->Reset());
        ASSERT_SUCCEEDED(s_copyCommandList->Reset(s_copyAllocator.Get(), nullptr));

        s_copyCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                dest.GetResource(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_DEST
            )
        );

        s_copyCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                src.GetResource(),
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_COPY_SOURCE
            )
        );

        s_copyCommandList->CopyResource(dest.GetResource(), src.GetResource());

        s_copyCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                dest.GetResource(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON
            )
        );

        s_copyCommandList->ResourceBarrier(
            1, &CD3DX12_RESOURCE_BARRIER::Transition(
                src.GetResource(),
                D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_COMMON
            )
        );

        ASSERT_SUCCEEDED(s_copyCommandList->Close());
    }
}
