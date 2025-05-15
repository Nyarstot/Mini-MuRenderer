#pragma once

#include "PixelBuffer.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class SharedResource : public PixelBuffer
    {
    private:
        PixelBuffer* m_pSharedResource;
        ComPtr<ID3D12Heap> m_primaryHeap;
        ComPtr<ID3D12Heap> m_sharedHeap;

    private:
        void CreateInternalTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap,
            const D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr);

        void CreateInternalSharedTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap,
            const D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, D3D12_CLEAR_VALUE ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr);

    public:
        SharedResource();
        ~SharedResource() = default;

        void Create(const std::wstring& Name, const D3D12_RESOURCE_DESC& ResourceDesc,
            D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

        PixelBuffer* GetSharedResource() const { return m_pSharedResource; }
        PixelBuffer& GetSharedResourceRef() const { return *m_pSharedResource; }
        ID3D12Resource& GetResourceRef() const { return *m_pResource.Get(); }

    };
}
