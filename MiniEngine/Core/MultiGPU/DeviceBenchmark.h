#pragma once


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class DeviceBenchmarkProvider final
    {
    private:
        ComPtr<ID3D12Resource> m_outputTexture;
        ComPtr<ID3D12DescriptorHeap> m_uavHeap;
        ComPtr<ID3D12Resource> m_timeCounterBuffer;
        ComPtr<ID3D12Resource> m_timeCounterReadback;

        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        ID3D12Device* m_parentDevice;
        UINT m_result;

    public:
        DeviceBenchmarkProvider() = default;
        ~DeviceBenchmarkProvider() = default;

        void Release();
        void Initialize(ID3D12Device* Device);
        UINT PerformBenchmark(float benchmarkTimeMs);
        UINT GetBenchmarkResult() const;

    };
}