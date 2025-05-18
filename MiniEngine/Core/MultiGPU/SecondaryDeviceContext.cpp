#include "pch.h"
#include "MultiGPU/SecondaryDeviceContext.h"



namespace MultiGPU
{
    void SecondaryDeviceContext::Create(ID3D12Device* Device)
    {
        m_parentDevice = Device;

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ASSERT_SUCCEEDED(m_parentDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_deviceCommandQueue)));
        ASSERT_SUCCEEDED(m_deviceCommandQueue->GetTimestampFrequency(&m_commandQueueFrequency));
        ASSERT_SUCCEEDED(m_parentDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_deviceCommandAllocator)));

        ASSERT_SUCCEEDED(m_parentDevice->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_deviceCommandAllocator.Get(), nullptr,
            IID_PPV_ARGS(&m_deviceCommandList)
        ));
    }
}
