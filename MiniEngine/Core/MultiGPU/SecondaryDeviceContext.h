#pragma once


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class SecondaryDeviceContext final
    {
    private:
        ID3D12Device* m_parentDevice;
        ComPtr<ID3D12CommandAllocator> m_deviceCommandAllocator;
        ComPtr<ID3D12CommandQueue> m_deviceCommandQueue;
        ComPtr<ID3D12GraphicsCommandList> m_deviceCommandList;

        UINT64 m_commandQueueFrequency;

    public:
        SecondaryDeviceContext() = default;
        ~SecondaryDeviceContext() = default;

        void Create(ID3D12Device* Device);

        ID3D12Device* GetParentDevice() const { return m_parentDevice; }

        ID3D12CommandAllocator* GetDirectCommandAllocator() const { return m_deviceCommandAllocator.Get(); };
        ID3D12CommandAllocator* const* GetDirectCommandAllocatorAddress() { return m_deviceCommandAllocator.GetAddressOf(); };

        ID3D12CommandQueue* GetDirectCommandQueue() const { return m_deviceCommandQueue.Get(); }
        ID3D12CommandQueue* const* GetDirectCommandQueueAddress() { return m_deviceCommandQueue.GetAddressOf(); }

        ID3D12GraphicsCommandList* GetDirectCommandList() const { return m_deviceCommandList.Get(); }
        ID3D12GraphicsCommandList* const* GetDirectCommandListAddress() { return m_deviceCommandList.GetAddressOf(); }

    };
}
