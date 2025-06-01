#pragma once

#include "GpuResource.h"
#include "BufferManager.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class SharedResource final : public GpuResource
    {
    private:
        GpuResource* m_pSharedResource;
        ComPtr<ID3D12Heap> m_primaryHeap;
        ComPtr<ID3D12Heap> m_sharedHeap;


    private:
        void CreateInternalTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, UINT HeapOffset,
            D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, const D3D12_CLEAR_VALUE* ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr);

        void CreateInternalSharedTextureResource(ID3D12Device* Device, const std::wstring& name, ID3D12Heap* Heap, UINT HeapOffset,
            D3D12_RESOURCE_DESC& ResourceDesc, const D3D12_RESOURCE_STATES& InitialState, const D3D12_CLEAR_VALUE* ClearValue, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr);

    public:
        SharedResource();
        ~SharedResource();

        void Create(ID3D12Device* PrimaryDevice, ID3D12Device* SecondaryDevice, const std::wstring& Name,
            D3D12_RESOURCE_DESC& ResourceDesc, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

        GpuResource* GetSharedResource() const { return m_pSharedResource; }
        GpuResource& GetSharedResourceRef() const { return *m_pSharedResource; }
        //ID3D12Resource& GetResourceRef() const { return *m_pResource.Get(); }

    };
}