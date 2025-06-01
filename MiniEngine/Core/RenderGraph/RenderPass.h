#pragma once

#include "RenderGraph/Base/Span.h"
#include "RenderGraph/RenderGraphRegistry.h"


using namespace Microsoft::WRL;
namespace RenderGraph
{
    enum class RGCrossAdapterIndex
    {
        Primary = 0x00,
        Secondary = 0x01
    };

    struct RGCrossAdapterInfo
    {
        RGCrossAdapterIndex execAdapterIndex;
        std::vector<RGCrossAdapterIndex> dependentAdapters;
        bool multiGPUAllowed = false;
        ComPtr<ID3D12Fence> sharedFence;
    };

    class RenderGraph;
    class RenderPass
    {
    private:
        std::wstring m_name;

    protected:
        RenderGraph* m_parentGraph;
        std::unordered_map<std::wstring, ResourceEntry> m_reads;
        std::unordered_map<std::wstring, ResourceEntry> m_writes;
        RGCrossAdapterInfo m_crossAdapterInfo;

    protected:
        explicit RenderPass(const std::wstring& name, RenderGraph* parentRenderGraph);

    public:
        virtual ~RenderPass() = default;

        void SetMultiAdapterAllowed(bool allowed, RGCrossAdapterIndex adapter);
        bool IsMultiAdapterAllowed() const;
        void AddDependentAdapter(RGCrossAdapterIndex adapter);
        void SetSharedFence(ComPtr<ID3D12Fence> sharedFence);

        RenderPass& ReadFrom(Span<ResourceEntry> resources);
        RenderPass& WriteTo(Span<ResourceEntry* const> resources);
        const std::wstring& GetName() const;

        virtual void Setup() = 0;
        virtual void Execute(GraphicsContext& ctx) = 0;

    };
}