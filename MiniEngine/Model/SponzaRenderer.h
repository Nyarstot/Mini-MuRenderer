//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//

#pragma once

#include <d3d12.h>
#include "../Core/RenderGraph/RenderGraph.h"

class GraphicsContext;
class ShadowCamera;
class ModelH3D;
class ExpVar;

namespace Math
{
    class Camera;
    class Vector3;
}

namespace Sponza
{
    void Startup( Math::Camera& camera, bool useRenderGraph);
    void RenderGraphStartup();
    void Cleanup( void );

    void DepthPrePass( CommandContext& ctx );
    void ShadowPass( CommandContext& ctx );
    void MainRenderPass( CommandContext& ctx );
    void LightGridPass( CommandContext& ctx );
    void SSAOPass( CommandContext& ctx );
    void LightShadowPass( CommandContext& ctx );
    void RecalculateParticles( CommandContext& ctx );
    void GenerateCameraVelocityBuffer( CommandContext& ctx );
    void ResolveTAA( CommandContext& ctx );
    void RenderParticles( CommandContext& ctx );
    void RenderDOF( CommandContext& ctx );
    void RenderBlur( CommandContext& ctx );
    
    void SetupGraphicsState(GraphicsContext& gfxContext);

    void RenderScene(
        GraphicsContext& gfxContext,
        const Math::Camera& camera,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissor,
        bool skipDiffusePass = false,
        bool skipShadowMap = false );

    void RenderSceneRenderGraph(
        GraphicsContext& gfxContext,
        const Math::Camera& camera,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissor,
        bool skipDiffusePass = false,
        bool skipShadowMap = false);

    void RenderSceneRenderGraphStoraged(
        GraphicsContext& gfxContext,
        const Math::Camera& camera,
        const D3D12_VIEWPORT& viewport,
        const D3D12_RECT& scissor,
        bool skipDiffusePass = false,
        bool skipShadowMap = false);

    const ModelH3D& GetModel();

    //extern std::unique_ptr<RenderGraph::RenderGraph> g_renderGraph;
    extern RenderGraph::RenderGraph* g_renderGraph;
    extern RenderGraph::RenderGraphStoraged* g_renderGraphStoraged;
    extern Math::Vector3 m_SunDirection;
    extern Math::Camera m_Camera;
    extern D3D12_VIEWPORT m_Viewport;
    extern D3D12_RECT m_Scissor;
    extern std::uint32_t m_FrameIndex;
    extern ShadowCamera m_SunShadow;
    extern ExpVar m_AmbientIntensity;
    extern ExpVar m_SunLightIntensity;

}
