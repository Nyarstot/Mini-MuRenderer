#pragma once

#include "RenderGraph/RenderGraphResource.h"
#include "GpuResource.h"


namespace RenderGraph
{
    struct ResourceEntry
    {
        std::shared_ptr<RenderGraphResource> resource;
        RenderGraphResourceType type;
        UINT64 sizeInBytes;
    };

    class RenderGraphRegistry final
    {
    private:
        std::unordered_map<std::wstring, ResourceEntry> m_resourceRegistry;

    public:
        RenderGraphRegistry() = default;
        ~RenderGraphRegistry() = default;

        ResourceEntry& RegisterResource(const std::wstring& name, GpuResource* resource, RenderGraphResourceType type);
        bool IsResourceRegistered(const std::wstring& name) const;

        std::shared_ptr<RenderGraphResource> GetRegisteredResource(const std::wstring& name) const;
        UINT64 GetRegisteredResourceSize(const std::wstring& name) const;

    };

    struct ResourceEntryHash {
        size_t operator()(const ResourceEntry& entry) const {
            size_t hash = 0;
            // Combine hashes of all members
            hash ^= std::hash<decltype(entry.resource)>()(entry.resource) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.type)>()(entry.type) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.sizeInBytes)>()(entry.sizeInBytes) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    struct ResourceEntryEqual {
        bool operator()(const ResourceEntry& a, const ResourceEntry& b) const {
            return a.resource == b.resource &&
                a.type == b.type &&
                a.sizeInBytes == b.sizeInBytes;
        }
    };
}