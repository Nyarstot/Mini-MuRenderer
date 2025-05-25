#pragma once

#include "RenderGraph/RenderGraphResource.h"
#include "GraphicsCore.h"


namespace RenderGraph
{
    namespace Utils
    {
        inline UINT64 CalculateResourceSize(D3D12_RESOURCE_DESC resourceDesc) {
            UINT64 size = 0;
            Graphics::g_Device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &size);
            return size;
        }
    }
}
