#pragma once


namespace RenderGraph
{
    class RenderGraph;
    class RenderPass
    {
    private:
        std::wstring m_name;

    protected:
        explicit RenderPass(const std::wstring& name);

    public:
        virtual ~RenderPass() = default;

        virtual void Setup(RenderGraph& renderGraph) = 0;
        virtual void Execute(ID3D12GraphicsCommandList* cmdList) = 0;

        const std::wstring& GetName() const;

    };

}