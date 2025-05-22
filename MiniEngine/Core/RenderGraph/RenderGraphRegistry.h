#pragma once

#include "RenderGraph/RenderGraphResource.h"

#include "DepthBuffer.h"
#include "ColorBuffer.h"
#include "ShadowBuffer.h"
#include "GpuBuffer.h"


namespace RenderGraph
{
    struct ResourceEntry
    {
        RenderGraphCommonResourceType commonType    = RenderGraphCommonResourceType::Unknown;
        RenderGraphResourceType type                = RenderGraphResourceType::Unknown;
        UINT64 sizeInBytes                          = 0;
        std::size_t registryIndex                   = 0;
        std::wstring name                           = L"";
    };

    class RenderGraphRegistry final
    {
        using ResourceMap = std::unordered_map<std::wstring, ResourceEntry>;

    private:
        ResourceMap m_resourceRegistry;
        std::shared_ptr<std::vector<DepthBuffer*>> m_depthBuffers;

    public:


    };
}