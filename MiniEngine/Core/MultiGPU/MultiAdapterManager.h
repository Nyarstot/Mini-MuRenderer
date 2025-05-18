#pragma once


using namespace Microsoft::WRL;

namespace MultiGPU
{
    enum DeviceIndex
    {
        PRIMARY = 0x00,
        SECONDARY = 0x01
    };

    class MultiAdapterManager final
    {
    private:
        std::vector<ID3D12Device*> m_devices;
        std::vector<IDXGIAdapter1*> m_adapters;

        ComPtr<ID3D12Fence> m_sharedFences[2];
        HANDLE m_fenceEvents[2];
        UINT64 m_currentSharedFenceValue;

    public:
        MultiAdapterManager() = default;
        ~MultiAdapterManager() = default;

        void AppendDevice(ID3D12Device* device, IDXGIAdapter1* adapter);
        void CreateSharedFence();

        std::size_t GetDeviceCount() const;
        ID3D12Device* GetDevice(std::size_t index = 0) const;

    public:
        static bool CheckRowMajorTextureSupport(ID3D12Device* device);

    };
}