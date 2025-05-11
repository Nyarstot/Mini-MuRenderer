#include "pch.h"
#include "MultiGPU/CrossAdapterCopyEngine.h"


namespace MultiGPU
{
    void CrossAdapterCopyEngine::Initialize()
    {
        ID3D12Device* primaryDevice = m_multiAdapterManager.GetDevice();
        ID3D12Device* secondaryDevice = m_multiAdapterManager.GetDevice(1);
        if (!MultiAdapterManager::CheckRowMajorTextureSupport(secondaryDevice)) {
            throw std::runtime_error("Given device doesn't support CASO");
        }

        UINT64 textureSize = 0;
        D3D12_RESOURCE_DESC crossAdapterResourceDesc = {};
        crossAdapterResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
        crossAdapterResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_RESOURCE_ALLOCATION_INFO textureInfo = primaryDevice->GetResourceAllocationInfo(0, 1, &crossAdapterResourceDesc);
        textureSize = textureInfo.SizeInBytes;

        // Create a heap for sharing
        CD3DX12_HEAP_DESC heapDesc(
            textureSize, D3D12_HEAP_TYPE_DEFAULT, 0,
            D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
        );

        ASSERT_SUCCEEDED(primaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_crossAdapterResourceHeaps[0])));

        // Create heap handle
        HANDLE heapHandle = nullptr;
        ASSERT_SUCCEEDED(primaryDevice->CreateSharedHandle(
            m_crossAdapterResourceHeaps[0].Get(),
            nullptr, GENERIC_ALL, nullptr, &heapHandle
        ));

        ASSERT_SUCCEEDED(secondaryDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_crossAdapterResourceHeaps[1])));
        CloseHandle(heapHandle);

    }

    CrossAdapterCopyEngine::CrossAdapterCopyEngine(const MultiAdapterManager& multiAdapterManager)
        : m_multiAdapterManager(multiAdapterManager)
    {
    }

    CrossAdapterCopyEngine::~CrossAdapterCopyEngine()
    {
    }
}