#include "pch.h"
#include "RenderGraph/RenderGraphResource.h"


namespace RenderGraph
{
    RenderGraphResource::RenderGraphResource(const std::wstring& name, const RenderGraphResourceDesc& desc)
        : GpuResource(nullptr, desc.initialState), m_name(name), m_desc(desc)
    {
    }

    RenderGraphResource::RenderGraphResource(const std::wstring& name, GpuResource* resource)
        : m_name(name)
    {
        m_pResource = resource->GetResource();
    }

    const std::wstring& RenderGraphResource::GetName() const
    {
        return m_name;
    }

    const RenderGraphResourceDesc& RenderGraphResource::GetDescription() const
    {
        return m_desc;
    }

    bool RenderGraphResource::IsValid() const
    {
        return m_pResource != nullptr;
    }

    void RenderGraphResource::SetResource(ComPtr<ID3D12Resource> resource)
    {
        m_pResource = resource;
    }

    void RenderGraphResource::SetName(const std::wstring& name)
    {
        m_name = name;
        if (m_pResource) {
            m_pResource->SetName(name.c_str());
        }
    }

    void RenderGraphResource::Destroy()
    {
        GpuResource::Destroy();
    }

    RenderGraphEdgeResourceData::RenderGraphEdgeResourceData(std::shared_ptr<RenderGraphResource> res, D3D12_RESOURCE_STATES state)
        : m_resource(res), m_requiredState(state)
    {
    }

    nlohmann::json RenderGraphEdgeResourceData::Serialize() const
    {
        nlohmann::json json;
        json["required_state"] = static_cast<int>(m_requiredState);
        json["resource"] = (m_resource != nullptr) ? m_resource->GetName() : nullptr;

        return json;
    }

    bool RenderGraphEdgeResourceData::IsValid() const
    {
        return m_resource != nullptr;
    }
}