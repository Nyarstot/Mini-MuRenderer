#pragma once

#include "GraphicsCore.h"
#include "GpuResource.h"
#include "MultiGPU/MultiAdapterManager.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class CrossAdapterCopyEngine final
    {
    private:
        bool m_rowMajorTextureSupport;
        MultiAdapterManager m_multiAdapterManager;

    private:
        void Initialize();

    public:
        CrossAdapterCopyEngine(const MultiAdapterManager& multiAdapterManager, bool rowMajorTextureSupport);
        ~CrossAdapterCopyEngine() = default;

        void CopyResource(
            ID3D12GraphicsCommandList* commandList,
            GpuResource* sourceResource,
            GpuResource* destinationResource
        );

    };
}