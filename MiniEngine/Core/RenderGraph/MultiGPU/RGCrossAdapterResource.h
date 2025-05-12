#pragma once

#include "RenderGraph/RenderGraphResource.h"
#include "MultiGPU/CrossAdapterCopyEngine.h"


namespace RenderGraph
{
    namespace MultiGPU
    {
        struct RGCrossAdapterResourceDesc
        {

        };

        class RGCrossAdapterResource : public RenderGraphResource
        {
        private:
            GpuResource* m_pSecondaryResource;
            ComPtr<ID3D12Heap> m_primaryHeap;
            ComPtr<ID3D12Heap> m_secondaryHeap;

        public:
            RGCrossAdapterResource(
                const std::wstring& name,
                const D3D12_RESOURCE_DESC& desc,
                D3D12_RESOURCE_STATES initialState
            );

            virtual ~RGCrossAdapterResource() = default;
            GpuResource* GetSecondaryResource() const;

        };
    }
}