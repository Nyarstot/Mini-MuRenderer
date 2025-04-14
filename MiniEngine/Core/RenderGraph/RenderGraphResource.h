#pragma once

using Microsoft::WRL::ComPtr;
namespace RenderGraph
{
    enum class RenderGraphResourceType
    {
        Unknown,
        Buffer,
        RTV,
        DSV,
        UAV
    };

    struct RenderGraphResourceDesc
    {
        RenderGraphResourceType type        = RenderGraphResourceType::Unknown;
        DXGI_FORMAT format                  = DXGI_FORMAT_UNKNOWN;
        std::uint32_t width                 = 0;
        std::uint32_t height                = 0;
        std::uint16_t depth                 = 1;
        std::uint16_t mipLevels             = 1;
        std::uint16_t arraySize             = 1;
        std::uint32_t sampleCount           = 1;
        D3D12_RESOURCE_FLAGS flags          = D3D12_RESOURCE_FLAG_NONE;
        D3D12_RESOURCE_STATES initialState  = D3D12_RESOURCE_STATE_COMMON;
    };

    class RenderGraphResource
    {
    private:
        std::wstring m_name;
        RenderGraphResourceDesc m_desc;
        ComPtr<ID3D12Resource> m_resource;

    public:
        RenderGraphResource(const std::wstring& name, const RenderGraphResourceDesc& desc);
        virtual ~RenderGraphResource() = default;

        const std::wstring& GetName() const;
        const RenderGraphResourceDesc& GetDescription() const;

        ComPtr<ID3D12Resource> GetResource() const;
        void SetResource(ComPtr<ID3D12Resource> resource);

    };

    struct RenderGraphEdgeResourceData
    {
        std::shared_ptr <RenderGraphResource> resource;
        D3D12_RESOURCE_STATES requiredState;
    };

}