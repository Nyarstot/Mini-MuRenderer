#include "pch.h"
#include "MultiGPU/CopyEngine.h"



namespace MultiGPU
{
    ComPtr<ID3D12CommandAllocator> CopyEngine::s_copyAllocator = nullptr;
    ComPtr<ID3D12CommandQueue> CopyEngine::s_copyCommandQueue = nullptr;
    ComPtr<ID3D12GraphicsCommandList> CopyEngine::s_copyCommandList = nullptr;

    void CopyEngine::Initialize(MultiAdapterManager& multiAdapterManager)
    {
        ID3D12Device* primaryDevice = multiAdapterManager.GetDevice();
        ID3D12Device* secondaryDevice = multiAdapterManager.GetDevice(1);

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

        s_copyCommandList->CopyResource(dest.GetResource(), src.GetResource());
        ASSERT_SUCCEEDED(s_copyCommandList->Close());
    }
}
