#include "pch.h"
#include "MultiGPU/SharedResource.h"
#include "Util/D3D12Utils.h"


namespace MultiGPU
{
    void SharedResource::CreateInternalTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, UINT HeapOffset, D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, const D3D12_CLEAR_VALUE* ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        this->Destroy();

        (void)VidMemPtr;
        ASSERT_SUCCEEDED(Device->CreatePlacedResource(
            Heap,
            HeapOffset,
            &ResourceDesc,
            InitialState,
            ClearValue,
            IID_PPV_ARGS(m_pResource.GetAddressOf())
        ));

        this->SetInitialState(InitialState, VidMemPtr);
    }

    void SharedResource::CreateInternalSharedTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, UINT HeapOffset, D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, const D3D12_CLEAR_VALUE* ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        m_pSharedResource->Destroy();

        (void)VidMemPtr;
        ASSERT_SUCCEEDED(Device->CreatePlacedResource(
            Heap,
            HeapOffset,
            &ResourceDesc,
            InitialState,
            ClearValue,
            IID_PPV_ARGS(m_pSharedResource->GetAddressOf())
        ));

        m_pSharedResource->SetInitialState(InitialState, VidMemPtr);
    }

    SharedResource::SharedResource()
        : GpuResource(), m_pSharedResource(new GpuResource())
    {
    }

    SharedResource::~SharedResource()
    {
        this->Destroy();
        m_pSharedResource->Destroy();
    }

    void SharedResource::Create(ID3D12Device* PrimaryDevice, ID3D12Device* SecondaryDevice, const std::wstring& Name, D3D12_RESOURCE_DESC& ResourceDesc, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        ResourceDesc.Flags                 = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
        ResourceDesc.Layout                = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        PrimaryDevice->GetCopyableFootprints(&ResourceDesc, 0, 1, 0, &layout, nullptr, nullptr, nullptr);
        auto textureSize = D3D12Utils::Align(layout.Footprint.RowPitch * layout.Footprint.Height);

        //CD3DX12_HEAP_DESC heapDesc(
        //    textureSize,
        //    D3D12_HEAP_TYPE_DEFAULT,
        //    0,
        //    D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
        //);

        CD3DX12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes = textureSize;
        heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapDesc.Properties.CreationNodeMask = 1;
        heapDesc.Properties.VisibleNodeMask = 1;
        heapDesc.Flags = D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;

        ASSERT_SUCCEEDED(PrimaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_primaryHeap)));

        HANDLE heapHandle = nullptr;
        ASSERT_SUCCEEDED(PrimaryDevice->CreateSharedHandle(
            m_primaryHeap.Get(),
            nullptr,
            GENERIC_ALL,
            nullptr,
            &heapHandle
        ));

        HRESULT openSharedHandleResult = SecondaryDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_sharedHeap));
        CloseHandle(heapHandle);
        ASSERT_SUCCEEDED(openSharedHandleResult);

        CreateInternalTextureResource(PrimaryDevice, Name, m_primaryHeap.Get(), 0, ResourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, VidMemPtr);
        CreateInternalSharedTextureResource(SecondaryDevice, Name + L"_Shared", m_sharedHeap.Get(), 0, ResourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, VidMemPtr);

        
    }
}