#include "pch.h"
#include "MultiGPU/SharedResource.h"

#include "Util/D3D12Utils.h"
#include "GraphicsCore.h"


namespace MultiGPU
{
    SharedResource::SharedResource()
        : PixelBuffer(), m_pSharedResource(new PixelBuffer())
    {
    }

    void SharedResource::CreateInternalTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, const D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        this->Destroy();

        (void)VidMemPtr;
        ASSERT_SUCCEEDED(Device->CreatePlacedResource(
            Heap,
            0,
            &ResourceDesc,
            InitialState,
            nullptr,
            IID_PPV_ARGS(&m_pResource)
        ));

        m_UsageState = InitialState;
        m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
    }

    void SharedResource::CreateInternalSharedTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, const D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        m_pSharedResource->Destroy();
        //auto resourceAddress = m_pSharedResource->GetAddressOf();

        (void)VidMemPtr;
        ASSERT_SUCCEEDED(Device->CreatePlacedResource(
            Heap,
            0,
            &ResourceDesc,
            InitialState,
            nullptr,
            IID_PPV_ARGS(m_pSharedResource->GetAddressOf())
        ));
    }

    void SharedResource::Create(const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        ID3D12Device* primaryDevice = Graphics::g_multiAdapterManager.GetDevice();
        ID3D12Device* secondaryDevice = Graphics::g_multiAdapterManager.GetDevice(1);

        D3D12_RESOURCE_DESC crossAdapterDesc    = ResourceDesc;
        crossAdapterDesc.Flags                  = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
        crossAdapterDesc.Layout                 = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        crossAdapterDesc.SampleDesc.Count       = 1;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = DXGI_FORMAT_D16_UNORM;

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
        auto textureSize = D3D12Utils::Align(layout.Footprint.RowPitch * layout.Footprint.Height);

        CD3DX12_HEAP_DESC heapDesc(
            textureSize * 3,
            D3D12_HEAP_TYPE_DEFAULT,
            0,
            D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
        );

        ASSERT_SUCCEEDED(primaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_primaryHeap)));

        HANDLE heapHandle = nullptr;
        ASSERT_SUCCEEDED(primaryDevice->CreateSharedHandle(
            m_primaryHeap.Get(),
            nullptr,
            GENERIC_ALL,
            nullptr,
            &heapHandle
        ));

        HRESULT openSharedHandleResult = secondaryDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_sharedHeap));
        CloseHandle(heapHandle);
        ASSERT_SUCCEEDED(openSharedHandleResult);

        CreateInternalTextureResource(primaryDevice, Name, m_primaryHeap.Get(), crossAdapterDesc, D3D12_RESOURCE_STATE_COPY_DEST, ClearValue, VidMemPtr);
        CreateInternalSharedTextureResource(secondaryDevice, Name + L"_Shared", m_sharedHeap.Get(), crossAdapterDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, ClearValue, VidMemPtr);
    }
}
