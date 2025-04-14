#include "pch.h"
#include "RenderGraph/RenderGraphResource.h"


namespace RenderGraph
{
    RenderGraphResource::RenderGraphResource(const std::wstring& name, const RenderGraphResourceDesc& desc)
        : m_name(name), m_desc(desc)
    {
    }

    const std::wstring& RenderGraphResource::GetName() const
    {
        return m_name;
    }

    const RenderGraphResourceDesc& RenderGraphResource::GetDescription() const
    {
        return m_desc;
    }

    ComPtr<ID3D12Resource> RenderGraphResource::GetResource() const
    {
        return m_resource;
    }

    void RenderGraphResource::SetResource(ComPtr<ID3D12Resource> resource)
    {
        m_resource = resource;
    }

    nlohmann::json RenderGraphEdgeResourceData::Serialize() const
    {
        nlohmann::json json;
        json["required_state"] = static_cast<int>(requiredState);
        (resource != nullptr) ? json["resource"] = resource->GetName() : json["resource"] = nullptr;

        return json;
    }
}