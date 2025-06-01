#include "pch.h"
#include "MultiGPU/CrossAdapterResource.h"


namespace MultiGPU
{
    void CrossAdapterResource::Create(ID3D12Device* PrimaryDevice, ID3D12Device* SecondaryDevice, GpuResource& ParentResource)
    {
        D3D12_RESOURCE_DESC& sharedResourceDesc = ParentResource.GetResource()->GetDesc();
        sharedResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
        sharedResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_RESOURCE_ALLOCATION_INFO textureAllocInfo = PrimaryDevice->GetResourceAllocationInfo(0, 1, &sharedResourceDesc);
        UINT sizeInBytes = textureAllocInfo.SizeInBytes;

        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes = sizeInBytes;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDesc.Properties.CreationNodeMask = 1;
        heapDesc.Properties.VisibleNodeMask = 1;
        heapDesc.Flags = D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;

        ASSERT_SUCCEEDED(PrimaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(m_heaps[0].GetAddressOf())));

        HANDLE sharedHandle = nullptr;
        ASSERT_SUCCEEDED(PrimaryDevice->CreateSharedHandle(
            m_heaps[0].Get(),
            nullptr,
            GENERIC_ALL,
            nullptr,
            &sharedHandle
        ));

        ASSERT_SUCCEEDED(SecondaryDevice->OpenSharedHandle(sharedHandle, IID_PPV_ARGS(m_heaps[1].GetAddressOf())));
        CloseHandle(sharedHandle);

        m_primaryResource->CreateDerivedViews(Graphics::g_Device, DXGI_FORMAT_R16G16B16A16_UNORM, 1);
        m_secondaryResource->CreateDerivedViews(Graphics::g_SecondaryDevice, DXGI_FORMAT_R16G16B16A16_UNORM, 1);
    }

    void CrossAdapterResource::Reset()
    {
    }
}
