#pragma once

#include "MultiGPU/MultiAdapterManager.h"
#include "GpuResource.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class CopyEngine final
    {
    private:
        static ComPtr<ID3D12CommandAllocator> s_copyAllocator;
        static ComPtr<ID3D12CommandQueue> s_copyCommandQueue;
        static ComPtr<ID3D12GraphicsCommandList> s_copyCommandList;
        UINT64 m_currentCrossAdapterFenceValue;

    public:
        static void Initialize(MultiAdapterManager& multiAdapterManager);
        static void CopyResource(GpuResource& dest, GpuResource& src);
        static void WaitForGPU();

        static ComPtr<ID3D12CommandQueue> GetCopyQueue() { return s_copyCommandQueue; }
        static ComPtr<ID3D12GraphicsCommandList> GetCopyCommandList() { return s_copyCommandList; }

    };
}