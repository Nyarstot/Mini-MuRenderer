#include "pch.h"
#include "MultiGPU/CrossAdapterCopyEngine.h"


namespace MultiGPU
{
    void CrossAdapterCopyEngine::Initialize()
    {


    }

    CrossAdapterCopyEngine::CrossAdapterCopyEngine(const MultiAdapterManager& multiAdapterManager, bool rowMajorTextureSupport)
        : m_multiAdapterManager(multiAdapterManager), m_rowMajorTextureSupport(rowMajorTextureSupport)
    {
    }

    void CrossAdapterCopyEngine::CopyResource(ID3D12GraphicsCommandList* commandList, GpuResource* sourceResource, GpuResource* destinationResource)
    {
        if (m_rowMajorTextureSupport) {
            // If cross-adapter row-major textures are supported, simply copy the texture
            commandList->CopyResource(destinationResource->GetResource(), sourceResource->GetResource());
        }
        else
        {
            // If not supported, we need to copy with explicit layout
            D3D12_RESOURCE_DESC sourceDesc = sourceResource->GetResource()->GetDesc();
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT sourceLayout;

            ID3D12Device* primaryDevice = m_multiAdapterManager.GetDevice();
            primaryDevice->GetCopyableFootprints(
                &sourceDesc, 0, 1, 0, &sourceLayout, nullptr, nullptr, nullptr
            );

            CD3DX12_TEXTURE_COPY_LOCATION dest(destinationResource->GetResource(), sourceLayout);
            CD3DX12_TEXTURE_COPY_LOCATION src(sourceResource->GetResource(), 0);
            // TODO: Add copying itself
        }
    }
}