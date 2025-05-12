#include "pch.h"
#include "RenderGraph/MultiGPU/RGCrossAdapterResource.h"
#include "MultiGPU/MultiAdapterManager.h"
#include "GraphicsCore.h"


namespace RenderGraph
{
    namespace MultiGPU
    {
        RGCrossAdapterResource::RGCrossAdapterResource(const std::wstring& name, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState)
            : RenderGraphResource(name, this)
        {
            ID3D12Device* primaryDevice = Graphics::g_multiAdapterManager.GetDevice();
            ID3D12Device* secondaryDevice = Graphics::g_multiAdapterManager.GetDevice(1);

            bool supportsDirectAccess = ::MultiGPU::MultiAdapterManager::CheckRowMajorTextureSupport(secondaryDevice);
            D3D12_RESOURCE_ALLOCATION_INFO allocInfo = primaryDevice->GetResourceAllocationInfo(0, 1, &desc);
            UINT64 textureSize = allocInfo.SizeInBytes;


            /**
            * TODO: For non-texture support, we need to create a buffer with the same layout as the texture
            *
            * if (rowMajorTextureSupport)
            * {
            *
            * }
            *
            */

            // Create a heap that will be shared by both adapters
            {
                CD3DX12_HEAP_DESC heapDesc(
                    textureSize,
                    D3D12_HEAP_TYPE_DEFAULT,
                    0, D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
                );

                ASSERT_SUCCEEDED(primaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_primaryHeap)));

                // Create shared handle for the heap
                HANDLE heapHandle = nullptr;
                ASSERT_SUCCEEDED(primaryDevice->CreateSharedHandle(
                    m_primaryHeap.Get(),
                    nullptr,
                    GENERIC_ALL,
                    nullptr,
                    &heapHandle
                ));

                // Open the shared handle on the secondary adapter
                ASSERT_SUCCEEDED(secondaryDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_secondaryHeap)));
                CloseHandle(heapHandle);
            }

            // Create resource
            {
                D3D12_RESOURCE_DESC primaryDesc = desc;
                if (!supportsDirectAccess) {
                    primaryDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);
                }
                else
                {
                    primaryDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
                    primaryDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                }

                ASSERT_SUCCEEDED(primaryDevice->CreatePlacedResource(
                    m_primaryHeap.Get(), textureSize,
                    &primaryDesc, initialState,
                    nullptr, IID_PPV_ARGS(&m_pResource)
                ));

                ID3D12Resource* secondaryResource = m_pSecondaryResource->GetResource();
                ASSERT_SUCCEEDED(secondaryDevice->CreatePlacedResource(
                    m_secondaryHeap.Get(), textureSize,
                    &primaryDesc, supportsDirectAccess ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_COPY_SOURCE,
                    nullptr, IID_PPV_ARGS(&secondaryResource)
                ));
            }
        }

        GpuResource* RGCrossAdapterResource::GetSecondaryResource() const
        {
            return nullptr;
        }
    }
}
