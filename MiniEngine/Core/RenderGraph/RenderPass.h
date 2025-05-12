#pragma once

#include "CommandContext.h"
#include "RenderGraph/Base/Span.h"
#include "RenderGraph/RenderGraphRegistry.h"


using namespace Microsoft::WRL;
namespace RenderGraph
{
    class RenderGraph;
    class RenderPass
    {
    private:
        std::wstring m_name;

    protected:
        RenderGraph* m_renderGraph;
        std::unordered_map<std::wstring, ResourceEntry> m_readWrites;
        std::unordered_map<std::wstring, ResourceEntry> m_reads;
        std::unordered_map<std::wstring, ResourceEntry> m_writes;

        std::size_t m_execAdapterIndex;
        std::vector<std::size_t> m_dependentAdapters;
        bool m_multiAdapterAllowed = false;
        ComPtr<ID3D12Fence> m_sharedFence;

    protected:
        explicit RenderPass(const std::wstring& name, RenderGraph* renderGraph = nullptr);
        virtual void InternalExecute(CommandContext& ctx) = 0;
        virtual void InternalExecuteMultiAdapter(CommandContext& ctx) = 0;
        virtual void Setup(const GraphicsContext& ctx);

    public:
        virtual ~RenderPass() = default;

        void SetMultiAdapterAllowed(bool allowed, std::size_t adapterIndex);
        bool IsMultiAdapterAllowed() const;
        void AddDependentAdapter(std::size_t adapterIndex);
        void SetSharedFence(ComPtr<ID3D12Fence> fence);

        RenderPass& ReadFrom(Span<ResourceEntry> resources);
        RenderPass& WriteTo(Span<ResourceEntry* const> resources);
        //RenderPass& AddReadWrite(Span<ResourceEntry* const> resources); // TODO: Add read/write

        virtual void Execute(CommandContext& ctx);
        const std::wstring& GetName() const;

    };

}