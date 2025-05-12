#pragma once

#include "RenderGraph/RenderGraphResource.h"
#include "RenderGraph/Utils/RenderGraphUtils.h"
#include "BufferManager.h"


namespace RenderGraph
{
    struct ResourceEntry
    {
        RenderGraphCommonResourceType commonType = RenderGraphCommonResourceType::Unknonw;
        RenderGraphResourceType type = RenderGraphResourceType::Unknown;
        UINT64 sizeInBytes;
        std::size_t index;
        std::wstring name;
    };

    class RenderGraphRegistry final
    {
    private:
        std::unordered_map<std::wstring, ResourceEntry> m_resourceRegistry;
        std::shared_ptr<std::vector<DepthBuffer*>> m_depthBuffers;
        std::shared_ptr<std::vector<ColorBuffer*>> m_colorBuffers;
        std::shared_ptr<std::vector<ShadowBuffer*>> m_shadowBuffers;
        std::shared_ptr<std::vector<StructuredBuffer*>> m_structuredBuffers;
        std::shared_ptr<std::vector<ByteAddressBuffer*>> m_byteAddressBuffers;
        std::shared_ptr<std::vector<TypedBuffer*>> m_typedBuffers;

    protected:
        template<typename Ty>
        auto& GetResourceContainer() const {
            // GetResourceContainer returns copy of the object for some reason
            // So i wrap it in std::shared_ptr...

            if constexpr (std::is_same_v<Ty, DepthBuffer>) {
                return m_depthBuffers;
            }
            if constexpr (std::is_same_v<Ty, ColorBuffer>) {
                return m_colorBuffers;
            };
            if constexpr (std::is_same_v<Ty, ShadowBuffer>) {
                return m_shadowBuffers;
            }
            if constexpr (std::is_same_v<Ty, StructuredBuffer>) {
                return m_structuredBuffers;
            }
            if constexpr (std::is_same_v<Ty, ByteAddressBuffer>) {
                return m_byteAddressBuffers;
            }
            if constexpr (std::is_same_v<Ty, TypedBuffer>) {
                return m_typedBuffers;
            }

            throw std::runtime_error("Unknown resource type");
        }

    public:
        RenderGraphRegistry();
        ~RenderGraphRegistry() = default;

        template<typename Ty>
        ResourceEntry RegisterResource(const std::wstring& name, Ty* resource) {
            auto container = GetResourceContainer<Ty>();
            container->emplace_back(resource);

            ResourceEntry entry = {};
            entry.index = container->size() - 1;
            entry.sizeInBytes = Utils::CalculateResourceSize(resource->GetResource()->GetDesc());
            entry.name = name;

            m_resourceRegistry.insert({ name, entry });
            return entry;
        }

        template<typename Ty>
        Ty* GetRegisteredResource(const std::wstring& name) {
            if (this->IsResourceRegistered(name)) {
                ResourceEntry entry = m_resourceRegistry.at(name);
                auto container = GetResourceContainer<Ty>();
                auto resource = container->at(entry.index);

                return resource;
            }

            throw std::runtime_error("Given name doesn't exists in registry");
        }

        template<typename Ty>
        Ty* GetRegisteredResource(const ResourceEntry& entry) {
            if (this->IsResourceRegistered(entry.name)) {
                auto container = GetResourceContainer<Ty>();
                auto resource = container->at(entry.index);

                return resource;
            }

            throw std::runtime_error("Given name doesn't exists in registry");
        }

        bool IsResourceRegistered(const std::wstring& name) const;

        ResourceEntry GetRegisteredResourceEntry(const std::wstring& name) const;
        UINT64 GetRegisteredResourceSize(const std::wstring& name) const;

    };

    struct ResourceEntryHash {
        size_t operator()(const ResourceEntry& entry) const {
            size_t hash = 0;
            // Combine hashes of all members
            hash ^= std::hash<decltype(entry.commonType)>()(entry.commonType) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.type)>()(entry.type) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.sizeInBytes)>()(entry.sizeInBytes) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.index)>()(entry.index) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= std::hash<decltype(entry.name)>()(entry.name) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };

    struct ResourceEntryEqual {
        bool operator()(const ResourceEntry& a, const ResourceEntry& b) const {
            return  a.commonType == a.commonType    &&
                    a.type == b.type                &&
                    a.sizeInBytes == b.sizeInBytes  &&
                    a.index == b.index              &&
                    a.name == b.name;
        }
    };
}