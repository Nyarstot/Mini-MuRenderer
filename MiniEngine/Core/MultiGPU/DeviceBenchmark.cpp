#include "pch.h"
#include "MultiGPU/DeviceBenchmark.h"

#include "PipelineState.h"
#include "CommandContext.h"
#include "CommandListManager.h"
#include "Utility.h"


namespace MultiGPU
{
    void DeviceBenchmarkProvider::Release()
    {
        m_commandAllocator->Release();
        m_commandQueue->Release();
        m_commandList->Release();
    }

    void DeviceBenchmarkProvider::Initialize(ID3D12Device* Device)
    {
        m_parentDevice = Device;

        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            //queueDesc.NodeMask = 1;

            ASSERT_SUCCEEDED(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, MY_IID_PPV_ARGS(&m_commandAllocator)));
            ASSERT_SUCCEEDED(Device->CreateCommandQueue(&queueDesc, MY_IID_PPV_ARGS(&m_commandQueue)));
            m_commandQueue->SetName(L"DeviceBenchmarkProvider::m_commandQueue");

            ASSERT_SUCCEEDED(Device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(),
                nullptr, IID_PPV_ARGS(&m_commandList)
            ));

            ASSERT_SUCCEEDED(m_commandList->Close());
        }
    }

    UINT DeviceBenchmarkProvider::PerformBenchmark(float benchmarkTimeMs)
    {
        char deviceName[128] = {};
        UINT size = sizeof(deviceName);
        m_parentDevice->GetPrivateData(WKPDID_D3DDebugObjectName, &size, deviceName);

        Utility::Printf("Starting benchmark on GPU %s...\n", deviceName);

        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        queryHeapDesc.Count = 2;

        ID3D12QueryHeap* timestampQueryHeap = nullptr;
        ASSERT_SUCCEEDED(m_parentDevice->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&timestampQueryHeap)));

        ID3D12Resource* queryBuffer = nullptr;
        ASSERT_SUCCEEDED(m_parentDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(2 * sizeof(uint64_t)),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&queryBuffer)
        ));

        UINT64 gpuFrequency;
        m_commandQueue->GetTimestampFrequency(&gpuFrequency);

        UINT frameCount = 0;
        auto startTime = std::chrono::high_resolution_clock::now();

        while (true)
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float elapsedSec = std::chrono::duration<float>(currentTime - startTime).count();
            if (elapsedSec >= benchmarkTimeMs)
                break;

            m_commandList->EndQuery(timestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
            m_commandList->Dispatch(32, 32, 1);
            m_commandList->EndQuery(timestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);

            m_commandList->ResolveQueryData(
                timestampQueryHeap,
                D3D12_QUERY_TYPE_TIMESTAMP,
                0, 2, queryBuffer, 0
            );

            m_commandList->Close();
            ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists(1, cmdLists);

            ID3D12Fence* fence;
            m_parentDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
            m_commandQueue->Signal(fence, 1);
            if (fence->GetCompletedValue() < 1)
            {
                HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                fence->SetEventOnCompletion(1, event);
                WaitForSingleObject(event, INFINITE);
                CloseHandle(event);
            }

            frameCount++;
            m_commandList->Reset(m_commandAllocator.Get(), nullptr);
        }

        // Result
        double fps = frameCount / benchmarkTimeMs;
        Utility::Printf("GPU %s rendered %d frames per %.2f seconds (%.2f FPS)\n",
            deviceName, frameCount, benchmarkTimeMs, fps);

        return frameCount;
    }

}
