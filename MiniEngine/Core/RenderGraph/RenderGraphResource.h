#pragma once


namespace RenderGraph
{
    enum class RenderGraphResourceType
    {
        Unknown,
        Buffer,
        Texture,
        RTV,
        DSV,
        UAV,
        SRV,
        RootSignature,
        PipelineState
    };

    enum class RenderGraphCommonResourceType
    {
        Unknown,
        DepthBuffer,
        ColorBuffer,
        ShadowBuffer,
        StructuredBuffer,
        ByteAddressBuffer,
        TypedBuffer
    };
}
