#include "pch.h"
#include "RenderGraph/RenderGraphRegistry.h"


namespace RenderGraph
{
    RenderGraphRegistry::RenderGraphRegistry()
    {
        m_depthBuffers = std::make_shared<std::vector<DepthBuffer*>>();
        m_colorBuffers = std::make_shared<std::vector<ColorBuffer*>>();
        m_shadowBuffers = std::make_shared<std::vector<ShadowBuffer*>>();
        m_byteAddressBuffers = std::make_shared<std::vector<ByteAddressBuffer*>>();
        m_structuredBuffers = std::make_shared<std::vector<StructuredBuffer*>>();
    }

    bool RenderGraphRegistry::IsResourceRegistered(const std::wstring& name) const
    {
        auto it = m_resourceRegistry.find(name);
        return (it == m_resourceRegistry.end()) ? false : true;
    }

    ResourceEntry RenderGraphRegistry::GetRegisteredResourceEntry(const std::wstring& name) const
    {
        if (IsResourceRegistered(name)) {
            ResourceEntry entry = m_resourceRegistry.at(name);
            return entry;
        }

        throw std::runtime_error("Resource entry doesn't exists!");
    }

    UINT64 RenderGraphRegistry::GetRegisteredResourceSize(const std::wstring& name) const
    {
        if (IsResourceRegistered(name)) {
            ResourceEntry entry = m_resourceRegistry.at(name);
            return entry.sizeInBytes;
        }

        throw std::runtime_error("Resource entry doesn't exists!");
    }
}
