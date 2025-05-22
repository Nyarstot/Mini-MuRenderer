#pragma once


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class DeviceBenchmarkProvider final
    {
    private:
        ID3D12Resource* m_outputTexture;
        ID3D12DescriptorHeap* m_uavHeap;

        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        ComPtr<ID3D12Fence> m_fence;

        ID3D12Device* m_parentDevice;

    public:
        DeviceBenchmarkProvider() = default;
        ~DeviceBenchmarkProvider() = default;

        void Release();
        void Initialize(ID3D12Device* Device);
        UINT PerformBenchmark(float benchmarkTimeMs);

    };
}