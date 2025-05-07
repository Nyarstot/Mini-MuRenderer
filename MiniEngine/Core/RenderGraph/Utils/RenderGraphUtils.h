#pragma once

#include "RenderGraph/RenderGraphResource.h"
#include "GraphicsCore.h"


namespace RenderGraph
{
    namespace utils
    {
        /**
        * @brief converts render graph resource description to D3D12_RESOURCE_DESC.
        * @param[in] desc render graph resource description.
        * @returns D3D12_RESOURCE_DESC representation of render graph resource.
        */
        inline D3D12_RESOURCE_DESC ConvertRGResourceDescToD3D12Desc(const RenderGraphResourceDesc& desc)
        {
            D3D12_RESOURCE_DESC resourceDesc    = {};
            resourceDesc.Dimension              = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // Most common case
            resourceDesc.Alignment              = 0;
            resourceDesc.Width                  = desc.width;
            resourceDesc.Height                 = desc.height;
            resourceDesc.DepthOrArraySize       = static_cast<UINT16>(desc.arraySize);
            resourceDesc.MipLevels              = desc.mipLevels;
            resourceDesc.Format                 = desc.format;
            resourceDesc.SampleDesc.Count       = desc.sampleCount;
            resourceDesc.SampleDesc.Quality     = 0;
            resourceDesc.Layout                 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            resourceDesc.Flags                  = desc.flags;

            // Special case for buffers
            if (desc.type == RenderGraphResourceType::Buffer)
            {
                resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
                resourceDesc.Width              = desc.width; // For buffers, width is size in bytes
                resourceDesc.Height             = 1;
                resourceDesc.DepthOrArraySize   = 1;
                resourceDesc.MipLevels          = 1;
                resourceDesc.Format             = DXGI_FORMAT_UNKNOWN;
                resourceDesc.SampleDesc.Count   = 1;
            }

            return resourceDesc;
        }

        /**
        * @brief calculates render graph resource size in bytes.
        * @param[in] desc render graph resource description.
        * @returns render graph resoure size in bytes.
        */
        inline UINT64 CalculateRGResourceSize(const RenderGraphResourceDesc& desc)
        {
            UINT64 size = 0;
            D3D12_RESOURCE_DESC resourceDesc = ConvertRGResourceDescToD3D12Desc(desc);
            Graphics::g_multiAdapterManager.GetDevice()->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &size);
            return size;
        }
    }
}