#include "pch.h"
#include "MultiGPU/MultiAdapterManager.h"


namespace MultiGPU
{
    void MultiAdapterManager::AppendDevice(ID3D12Device* device, IDXGIAdapter1* adapter)
    {
        m_devices.push_back(device);
        m_adapters.push_back(adapter);
    }

    void MultiAdapterManager::CreateSharedFence()
    {
        ID3D12Device* device = m_devices[0];
        device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&device));
    }

    std::size_t MultiAdapterManager::GetDeviceCount() const
    {
        return m_devices.size();
    }

    ID3D12Device* MultiAdapterManager::GetDevice(std::size_t index) const
    {
        return m_devices[index];
    }

    bool MultiAdapterManager::CheckRowMajorTextureSupport(ID3D12Device* device)
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
        ASSERT_SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options), sizeof(options)));
        return options.CrossAdapterRowMajorTextureSupported;
    }
}