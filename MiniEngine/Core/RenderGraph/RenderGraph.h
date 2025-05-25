#pragma once

#include "RenderGraph/Base/BaseGraph.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/RenderGraphResource.h"


namespace RenderGraph
{
    class RenderGraph final : public BaseGraph<RenderPass, RenderGraphEdgeResourceData>
    {
    private:
        std::wstring m_title;
        RenderGraphRegistry m_resourceRegistry;
        ComPtr<ID3D12Fence> m_sharedFence;
        UINT64 m_fenceValue = 1;

    public:
        explicit RenderGraph(const std::wstring& name = L"Untitled RenderGraph");
        ~RenderGraph() = default;

        ResourceEntry& GetRegisteredResourceEntry(const std::wstring& name) const;
        bool IsResourceRegistered(const std::wstring& name) const;

        void Setup();
        void Build();
        void Compile();
        void Execute(CommandContext& ctx);
        void Clear();

    public:
        template<typename Ty>
        ResourceEntry RegisterExternalResource(const std::wstring& name, Ty* resource) {
            return m_resourceRegistry.RegisterResource<Ty>(name, resource);
        }

        template<typename Ty>
        Ty* GetRegisteredResource(const std::wstring& name) {
            return m_resourceRegistry.GetRegisteredResource<Ty>(name);
        }


    };
}