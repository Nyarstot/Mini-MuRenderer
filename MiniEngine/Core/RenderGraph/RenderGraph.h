#pragma once

#include "json.hpp"

#include "RenderGraph/Base/BaseGraph.h"
#include "RenderGraph/RenderGraphResource.h"
#include "RenderGraph/RenderPass.h"


namespace RenderGraph
{
    class RenderGraph final : public BaseGraph<RenderPass, RenderGraphEdgeResourceData>
    {
    private:
        std::unordered_map<std::wstring, std::shared_ptr<RenderGraphResource>> m_resources;
        ComPtr<ID3D12Fence> m_sharedFence;
        UINT64 m_fenceValue = 1;

    public:
        RenderGraph();

        std::shared_ptr<RenderGraphResource> CreateResource(const std::wstring& name, const RenderGraphResourceDesc& desc);
        std::shared_ptr<RenderGraphResource> GetResource(const std::wstring& name) const;

        void Compile();
        void Execute(CommandContext& ctx);
        void Clear();

        void AssignPassToAdapter(const std::wstring& passName, std::size_t adapterIndex, const std::vector<std::size_t>& dependentAdapters = {});

        nlohmann::json Serialize() const;
        void ExportToJSON() const;

        D3D12_RESOURCE_STATES GetCurrentState(const std::shared_ptr<RenderGraphResource>& resource) const;

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