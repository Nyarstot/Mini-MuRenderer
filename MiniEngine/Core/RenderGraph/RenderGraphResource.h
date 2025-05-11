#pragma once

#include "json.hpp"
#include "GpuResource.h"


using Microsoft::WRL::ComPtr;
namespace RenderGraph
{
    /**
    * @enum RenderGraphResourceType
    * @brief describes types of the resource supported by the render graph.
    */
    enum class RenderGraphResourceType
    {
        /**
        * Just in case you forget :)
        *
        * RTV - Render Target View
        * DSV - Depth Stencil View
        * UAV - Unordered Access View
        * SRV - Shader Resource View
        */

        Unknown,
        Buffer,
        Texture,
        RTV,
        DSV,
        UAV,
        SRV,
        RootSignature,
        PipelineState,
        CrossAdapterBuffer,
        CrossAdapterTexture
    };

    struct RenderGraphResourceDesc
    {
        RenderGraphResourceType type = RenderGraphResourceType::Unknown;
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

    /**
    * @class RenderGraphResource RenderGraphResource.h "Core/RenderGraph/RenderGraphResource.h"
    * @brief provides interface to interact with all the resources stored in render graph.
    */
    class RenderGraphResource : public GpuResource
    {
    private:
        std::wstring m_name;
        RenderGraphResourceDesc m_desc;

    public:
        RenderGraphResource(const std::wstring& name, const RenderGraphResourceDesc& desc);
        RenderGraphResource(const std::wstring& name, GpuResource* resource);
        virtual ~RenderGraphResource() = default;

        const std::wstring& GetName() const;
        const RenderGraphResourceDesc& GetDescription() const;

        bool IsValid() const;
        void SetResource(ComPtr<ID3D12Resource> resource);
        void SetName(const std::wstring& name);
        virtual void Destroy() override;

    };

    class RenderGraphEdgeResourceData
    {
    public:
        std::shared_ptr<RenderGraphResource> m_resource;
        D3D12_RESOURCE_STATES m_requiredState;

    public:
        RenderGraphEdgeResourceData() = default;
        RenderGraphEdgeResourceData(std::shared_ptr<RenderGraphResource> res, D3D12_RESOURCE_STATES state);
        ~RenderGraphEdgeResourceData() = default;

        nlohmann::json Serialize() const;
        bool IsValid() const;

    };

}