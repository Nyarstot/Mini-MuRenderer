#pragma once

#include "json.hpp"

#include "RenderGraph/Base/BaseGraph.h"
#include "RenderGraph/RenderGraphResource.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/RenderPass.h"


namespace RenderGraph
{
    /**
    * @class RenderGraph RenderGraph.h "Core/RenderGraph/RenderGraph.h"
    * @brief general render graph representation class.
    */
    class RenderGraph final : public BaseGraph<RenderPass, RenderGraphEdgeResourceData>
    {
    private:
        RenderGraphRegistry m_resourceRegistry;
        std::unordered_map<std::wstring, std::shared_ptr<RenderGraphResource>> m_resources;
        ComPtr<ID3D12Fence> m_sharedFence;
        UINT64 m_fenceValue = 1;

    public:
        RenderGraph(const std::wstring& name = L"");

        std::size_t AddRenderPass(std::unique_ptr<RenderPass> renderPass);

        ResourceEntry GetRegisteredResourceEntry(const std::wstring& name) const;

        /**
        * @brief checks if resource with given name already registered.
        * @param[in] name resource name in registry.
        * @returns true if resource registered, otherwise false.
        */
        bool IsResourceRegistered(const std::wstring& name) const;

        std::shared_ptr<RenderGraphResource> CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc);
        std::shared_ptr<RenderGraphResource> GetResource(const std::wstring& name) const;

        void Build();

        /**
        * @brief perform render graph optimizations and checks before executions
        */
        void Compile();

        /**
        * @brief executes render graph nodes pass by pass
        * @param[in] ctx mini engine CommandContext
        */
        void Execute(CommandContext& ctx);

        /**
        * @brief clears all nodes and their dependencies in graph
        */
        void Clear();

        nlohmann::json Serialize() const;
        void ExportToJSON() const;

        D3D12_RESOURCE_STATES GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const;

    public:
        /**
        * @brief registers externally created resource in render graph registry.
        * @param[in] name resource name to set under registry.
        * @param[in] resource external resource.
        * @returns resource entry describes resource in registry;
        */
        template<typename Ty>
        ResourceEntry RegisterExternalResource(const std::wstring& name, Ty* resource) {
            return m_resourceRegistry.RegisterResource<Ty>(name, resource);
        }


        /**
        * @brief gets previously registered gpu resource.
        * @param[in] name name of the resource in registry.
        * @tparam Ty type of the resource.
        * @returns resource lying under given name in render graph registry.
        */
        template<typename Ty>
        Ty* GetRegisteredResource(const std::wstring& name) {
            return m_resourceRegistry.GetRegisteredResource<Ty>(name);
        }


    };

    class RenderGraphStoraged final : public BaseGraphStoraged<RenderPass, RenderGraphEdgeResourceData>
    {
    private:
        std::unordered_map<std::wstring, std::shared_ptr<RenderGraphResource>> m_resources;

    public:
        RenderGraphStoraged();

        std::shared_ptr<RenderGraphResource> CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc);
        std::shared_ptr<RenderGraphResource> GetResource(const std::wstring& name) const;

        void Compile();
        void Execute(CommandContext& ctx);
        void Clear();

        nlohmann::json Serialize() const;
        void ExportToJSON() const;

        static RenderGraphStoraged* Deserialize(const nlohmann::json& from);
        static RenderGraphStoraged* ImportFromJSON();

        D3D12_RESOURCE_STATES GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const;

    };

}