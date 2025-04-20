#pragma once


using namespace Microsoft::WRL;

namespace RenderGraph
{
    class RenderGraph;
    class RenderPass
    {
    private:
        std::wstring m_name;

    protected:
        std::size_t m_execAdapterIndex;
        std::vector<std::size_t> m_dependentAdapters;
        bool m_multiAdapterAllowed = false;
        ComPtr<ID3D12Fence> m_sharedFence;

    protected:
        explicit RenderPass(const std::wstring& name);
        virtual void InternalExecute(CommandContext& ctx) = 0;
        virtual void InternalExecuteMultiAdapter(CommandContext& ctx) = 0;

    public:
        virtual ~RenderPass() = default;

        void SetMultiAdapterAllowed(bool allowed, std::size_t adapterIndex);
        bool IsMultiAdapterAllowed() const;
        void AddDependentAdapter(std::size_t adapterIndex);
        void SetSharedFence(ComPtr<ID3D12Fence> fence);

        virtual void Setup(RenderGraph& renderGraph) = 0;
        virtual void Execute(CommandContext& ctx);

        const std::wstring& GetName() const;

    };

}