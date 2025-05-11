#include "pch.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/Utils/RenderGraphUtils.h"


namespace RenderGraph
{
    ResourceEntry& RenderGraphRegistry::RegisterResource(const std::wstring& name, GpuResource* resource, RenderGraphResourceType type)
    {
        auto rgResource = std::make_shared<RenderGraphResource>(name, resource);
        D3D12_RESOURCE_DESC resourceDesc = resource->GetResource()->GetDesc();

        ResourceEntry entry = {
            rgResource,
            type,
            Utils::CalculateResourceSize(resourceDesc)
        };
        m_resourceRegistry.insert({ name, entry});

        return entry;
    }

    bool RenderGraphRegistry::IsResourceRegistered(const std::wstring& name) const
    {
        auto it = m_resourceRegistry.find(name);
        return (it == m_resourceRegistry.end()) ? false : true;
    }

    std::shared_ptr<RenderGraphResource> RenderGraphRegistry::GetRegisteredResource(const std::wstring& name) const
    {
        return std::shared_ptr<RenderGraphResource>();
    }

    UINT64 RenderGraphRegistry::GetRegisteredResourceSize(const std::wstring& name) const
    {
        return UINT64();
    }
}