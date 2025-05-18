#include "pch.h"
#include "MultiGPU/MultiAdapterManager.h"

#include "GraphicsCore.h"
#include "CommandContext.h"


namespace MultiGPU
{
    void MultiAdapterManager::AppendDevice(ID3D12Device* device, IDXGIAdapter1* adapter)
    {
        m_devices.push_back(device);
        m_adapters.push_back(adapter);
    }

    void MultiAdapterManager::CreateSharedFence()
    {
        ID3D12Device* primary = m_devices[0];
        ID3D12Device* secondary = m_devices[1];

        ASSERT_SUCCEEDED(primary->CreateFence(
            0, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER,
            IID_PPV_ARGS(&m_sharedFences[DeviceIndex::PRIMARY])
        ));

        HANDLE fenceHandle = nullptr;
        ASSERT_SUCCEEDED(primary->CreateSharedHandle(
            m_sharedFences[DeviceIndex::PRIMARY].Get(),
            nullptr,
            GENERIC_ALL,
            nullptr,
            &fenceHandle
        ));

        HRESULT openSharedHandleResult = secondary->OpenSharedHandle(fenceHandle, IID_PPV_ARGS(&m_sharedFences[DeviceIndex::SECONDARY]));
        CloseHandle(fenceHandle);
        ASSERT_SUCCEEDED(openSharedHandleResult);

        for (UINT i = 0; i < 2; i++) {
            m_fenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvents == nullptr) {
                ASSERT_SUCCEEDED(HRESULT_FROM_WIN32(GetLastError()));
            }


            if (i == DeviceIndex::PRIMARY) {
                ASSERT_SUCCEEDED(Graphics::g_CommandManager.GetCommandQueue()->Signal(m_sharedFences[DeviceIndex::PRIMARY].Get(), m_currentSharedFenceValue));

                ASSERT_SUCCEEDED(m_sharedFences[DeviceIndex::PRIMARY]->SetEventOnCompletion(m_currentSharedFenceValue, m_fenceEvents[DeviceIndex::PRIMARY]));
                WaitForSingleObject(m_fenceEvents[DeviceIndex::PRIMARY], INFINITE);
                m_currentSharedFenceValue++;
            }
            else if (i == DeviceIndex::SECONDARY) {
                ASSERT_SUCCEEDED(Graphics::g_secondaryDeviceContext.GetDirectCommandQueue()->Signal(m_sharedFences[DeviceIndex::SECONDARY].Get(), m_currentSharedFenceValue));

                ASSERT_SUCCEEDED(m_sharedFences[DeviceIndex::SECONDARY]->SetEventOnCompletion(m_currentSharedFenceValue, m_fenceEvents[DeviceIndex::SECONDARY]));
                WaitForSingleObject(m_fenceEvents[DeviceIndex::SECONDARY], INFINITE);
                m_currentSharedFenceValue++;
            }
        }
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