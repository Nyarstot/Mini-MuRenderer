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
// Author(s):  Alex Nankervis
//             James Stanard
//

// From Core
#include "GraphicsCore.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "SystemTime.h"
#include "ShadowCamera.h"
#include "ParticleEffects.h"
#include "ParticleEffectManager.h"
#include "SponzaRenderer.h"
#include "Renderer.h"
#include "Display.h"

// From Model
#include "ModelH3D.h"

// From ModelViewer
#include "LightManager.h"

#include "CompiledShaders/DepthViewerVS.h"
#include "CompiledShaders/DepthViewerPS.h"
#include "CompiledShaders/ModelViewerVS.h"
#include "CompiledShaders/ModelViewerPS.h"

// Render graph
#include "../Core/RenderGraph/RenderPass.h"
#include "../Core/RenderGraph/RenderGraphResource.h"
#include "../Core/RenderGraph/Passes/LambdaRenderPass.h"

//#include "../Core/MultiGPU/CrossAdapterResource.h"
#include "../Core/MultiGPU/MultiAdapterManager.h"

using namespace Math;
using namespace Graphics;

namespace Sponza
{
    void RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera);

    enum eObjectFilter { kOpaque = 0x1, kCutout = 0x2, kTransparent = 0x4, kAll = 0xF, kNone = 0x0 };
    void RenderObjects( GraphicsContext& Context, const Matrix4& ViewProjMat, const Vector3& viewerPos, eObjectFilter Filter = kAll );

    GraphicsPSO m_DepthPSO = { (L"Sponza: Depth PSO") };
    GraphicsPSO m_CutoutDepthPSO = { (L"Sponza: Cutout Depth PSO") };
    GraphicsPSO m_ModelPSO = { (L"Sponza: Color PSO") };
    GraphicsPSO m_CutoutModelPSO = { (L"Sponza: Cutout Color PSO") };
    GraphicsPSO m_ShadowPSO(L"Sponza: Shadow PSO");
    GraphicsPSO m_CutoutShadowPSO(L"Sponza: Cutout Shadow PSO");

    ModelH3D m_Model;
    std::vector<bool> m_pMaterialIsCutout;

    // Can't use smart pointer for some reason. Object just keeps be nullptr
    // after initialization. May be i don't know something. By the way, it is
    // working with raw pointer and i'm ok with this.
    RenderGraph::RenderGraph* g_renderGraph;
    RenderGraph::RenderGraphStoraged* g_renderGraphStoraged;
    bool g_useMultiGPU = false;

    Vector3 m_SunDirection;
    ShadowCamera m_SunShadow;
    Math::Camera m_Camera;
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_Scissor;
    std::uint32_t m_FrameIndex;

    ExpVar m_AmbientIntensity("Sponza/Lighting/Ambient Intensity", 0.1f, -16.0f, 16.0f, 0.1f);
    ExpVar m_SunLightIntensity("Sponza/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
    NumVar m_SunOrientation("Sponza/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f );
    NumVar m_SunInclination("Sponza/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f );
    NumVar ShadowDimX("Sponza/Lighting/Shadow Dim X", 5000, 1000, 10000, 100 );
    NumVar ShadowDimY("Sponza/Lighting/Shadow Dim Y", 3000, 1000, 10000, 100 );
    NumVar ShadowDimZ("Sponza/Lighting/Shadow Dim Z", 3000, 1000, 10000, 100 );
}

void Sponza::Startup( Camera& Camera, bool useRenderGraph)
{
    DXGI_FORMAT ColorFormat = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT NormalFormat = g_SceneNormalBuffer.GetFormat();
    DXGI_FORMAT DepthFormat = g_SceneDepthBuffer.GetFormat();
    //DXGI_FORMAT ShadowFormat = g_ShadowBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Depth-only (2x rate)
    m_DepthPSO.SetRootSignature(Renderer::m_RootSig);
    m_DepthPSO.SetRasterizerState(RasterizerDefault);
    m_DepthPSO.SetBlendState(BlendNoColorWrite);
    m_DepthPSO.SetDepthStencilState(DepthStateReadWrite);
    m_DepthPSO.SetInputLayout(_countof(vertElem), vertElem);
    m_DepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_DepthPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
    m_DepthPSO.SetVertexShader(g_pDepthViewerVS, sizeof(g_pDepthViewerVS));
    m_DepthPSO.Finalize();

    // Depth-only shading but with alpha testing
    m_CutoutDepthPSO = m_DepthPSO;
    m_CutoutDepthPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutDepthPSO.Finalize();

    // Depth-only but with a depth bias and/or render only backfaces
    m_ShadowPSO = m_DepthPSO;
    m_ShadowPSO.SetRasterizerState(RasterizerShadow);
    m_ShadowPSO.SetRenderTargetFormats(0, nullptr, g_ShadowBuffer.GetFormat());
    m_ShadowPSO.Finalize();

    // Shadows with alpha testing
    m_CutoutShadowPSO = m_ShadowPSO;
    m_CutoutShadowPSO.SetPixelShader(g_pDepthViewerPS, sizeof(g_pDepthViewerPS));
    m_CutoutShadowPSO.SetRasterizerState(RasterizerShadowTwoSided);
    m_CutoutShadowPSO.Finalize();

    DXGI_FORMAT formats[2] = { ColorFormat, NormalFormat };

    // Full color pass
    m_ModelPSO = m_DepthPSO;
    m_ModelPSO.SetBlendState(BlendDisable);
    m_ModelPSO.SetDepthStencilState(DepthStateTestEqual);
    m_ModelPSO.SetRenderTargetFormats(2, formats, DepthFormat);
    m_ModelPSO.SetVertexShader( g_pModelViewerVS, sizeof(g_pModelViewerVS) );
    m_ModelPSO.SetPixelShader( g_pModelViewerPS, sizeof(g_pModelViewerPS) );
    m_ModelPSO.Finalize();

    m_CutoutModelPSO = m_ModelPSO;
    m_CutoutModelPSO.SetRasterizerState(RasterizerTwoSided);
    m_CutoutModelPSO.Finalize();

    ASSERT(m_Model.Load(L"Sponza/sponza.h3d"), "Failed to load model");
    ASSERT(m_Model.GetMeshCount() > 0, "Model contains no meshes");

    // The caller of this function can override which materials are considered cutouts
    m_pMaterialIsCutout.resize(m_Model.GetMaterialCount());
    for (uint32_t i = 0; i < m_Model.GetMaterialCount(); ++i)
    {
        const ModelH3D::Material& mat = m_Model.GetMaterial(i);
        if (std::string(mat.texDiffusePath).find("thorn") != std::string::npos ||
            std::string(mat.texDiffusePath).find("plant") != std::string::npos ||
            std::string(mat.texDiffusePath).find("chain") != std::string::npos)
        {
            m_pMaterialIsCutout[i] = true;
        }
        else
        {
            m_pMaterialIsCutout[i] = false;
        }
    }

    ParticleEffects::InitFromJSON(L"Sponza/particles.json");

    Sponza::RenderGraphStartup();

    float modelRadius = Length(m_Model.GetBoundingBox().GetDimensions()) * 0.5f;
    const Vector3 eye = m_Model.GetBoundingBox().GetCenter() + Vector3(modelRadius * 0.5f, 0.0f, 0.0f);
    Camera.SetEyeAtUp( eye, Vector3(kZero), Vector3(kYUnitVector) );

    Lighting::CreateRandomLights(m_Model.GetBoundingBox().GetMin(), m_Model.GetBoundingBox().GetMax());
}

const ModelH3D& Sponza::GetModel()
{
    return Sponza::m_Model;
}

void Sponza::Cleanup( void )
{
    m_Model.Clear();
    Lighting::Shutdown();
    TextureManager::Shutdown();
}

void Sponza::RenderObjects( GraphicsContext& gfxContext, const Matrix4& ViewProjMat, const Vector3& viewerPos, eObjectFilter Filter )
{
    struct VSConstants
    {
        Matrix4 modelToProjection;
        Matrix4 modelToShadow;
        XMFLOAT3 viewerPos;
    } vsConstants;
    vsConstants.modelToProjection = ViewProjMat;
    vsConstants.modelToShadow = m_SunShadow.GetShadowMatrix();
    XMStoreFloat3(&vsConstants.viewerPos, viewerPos);

    gfxContext.SetDynamicConstantBufferView(Renderer::kMeshConstants, sizeof(vsConstants), &vsConstants);

    __declspec(align(16)) uint32_t materialIdx = 0xFFFFFFFFul;

    uint32_t VertexStride = m_Model.GetVertexStride();

    for (uint32_t meshIndex = 0; meshIndex < m_Model.GetMeshCount(); meshIndex++)
    {
        const ModelH3D::Mesh& mesh = m_Model.GetMesh(meshIndex);

        uint32_t indexCount = mesh.indexCount;
        uint32_t startIndex = mesh.indexDataByteOffset / sizeof(uint16_t);
        uint32_t baseVertex = mesh.vertexDataByteOffset / VertexStride;

        if (mesh.materialIndex != materialIdx)
        {
            if ( m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kCutout) ||
                !m_pMaterialIsCutout[mesh.materialIndex] && !(Filter & kOpaque) )
                continue;

            materialIdx = mesh.materialIndex;
            gfxContext.SetDescriptorTable(Renderer::kMaterialSRVs, m_Model.GetSRVs(materialIdx));

            gfxContext.SetDynamicConstantBufferView(Renderer::kCommonCBV, sizeof(uint32_t), &materialIdx);
        }

        gfxContext.DrawIndexed(indexCount, startIndex, baseVertex);
    }
}

void Sponza::RenderLightShadows(GraphicsContext& gfxContext, const Camera& camera)
{
    using namespace Lighting;

    ScopedTimer _prof(L"RenderLightShadows", gfxContext);

    static uint32_t LightIndex = 0;
    if (LightIndex >= MaxLights)
        return;

    m_LightShadowTempBuffer.BeginRendering(gfxContext);
    {
        gfxContext.SetPipelineState(m_ShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], camera.GetPosition(), kOpaque);
        gfxContext.SetPipelineState(m_CutoutShadowPSO);
        RenderObjects(gfxContext, m_LightShadowMatrix[LightIndex], camera.GetPosition(), kCutout);
    }
    //m_LightShadowTempBuffer.EndRendering(gfxContext);

    gfxContext.TransitionResource(m_LightShadowTempBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_COPY_DEST);

    gfxContext.CopySubresource(m_LightShadowArray, LightIndex, m_LightShadowTempBuffer, 0);

    gfxContext.TransitionResource(m_LightShadowArray, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    ++LightIndex;
}

void Sponza::RenderScene(
    GraphicsContext& gfxContext,
    const Camera& camera,
    const D3D12_VIEWPORT& viewport,
    const D3D12_RECT& scissor,
    bool skipDiffusePass,
    bool skipShadowMap)
{
    Renderer::UpdateGlobalDescriptors();

    uint32_t FrameIndex = TemporalEffects::GetFrameIndexMod2();

    float costheta = cosf(m_SunOrientation);
    float sintheta = sinf(m_SunOrientation);
    float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
    float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
    m_SunDirection = Normalize(Vector3( costheta * cosphi, sinphi, sintheta * cosphi ));

    __declspec(align(16)) struct
    {
        Vector3 sunDirection;
        Vector3 sunLight;
        Vector3 ambientLight;
        float ShadowTexelSize[4];

        float InvTileDim[4];
        uint32_t TileCount[4];
        uint32_t FirstLightIndex[4];

		uint32_t FrameIndexMod2;
    } psConstants;

    psConstants.sunDirection = m_SunDirection;
    psConstants.sunLight = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    psConstants.ambientLight = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    psConstants.ShadowTexelSize[0] = 1.0f / g_ShadowBuffer.GetWidth();
    psConstants.InvTileDim[0] = 1.0f / Lighting::LightGridDim;
    psConstants.InvTileDim[1] = 1.0f / Lighting::LightGridDim;
    psConstants.TileCount[0] = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
    psConstants.TileCount[1] = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
    psConstants.FirstLightIndex[0] = Lighting::m_FirstConeLight;
    psConstants.FirstLightIndex[1] = Lighting::m_FirstConeShadowedLight;
	psConstants.FrameIndexMod2 = FrameIndex;

    // Set the default state for command lists
    auto& pfnSetupGraphicsState = [&](void)
    {
        gfxContext.SetRootSignature(Renderer::m_RootSig);
        gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Renderer::s_TextureHeap.GetHeapPointer());
        gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
        gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
    };

    pfnSetupGraphicsState();

    RenderLightShadows(gfxContext, camera);

    {
        ScopedTimer _prof(L"Z PrePass", gfxContext);

        gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

        {
            ScopedTimer _prof2(L"Opaque", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
                gfxContext.ClearDepth(g_SceneDepthBuffer);
                gfxContext.SetPipelineState(m_DepthPSO);
                gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
                gfxContext.SetViewportAndScissor(viewport, scissor);
            }
            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kOpaque );
        }

        {
            ScopedTimer _prof2(L"Cutout", gfxContext);
            {
                gfxContext.SetPipelineState(m_CutoutDepthPSO);
            }
            RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kCutout );
        }
    }

    SSAO::Render(gfxContext, camera);

    if (!skipDiffusePass)
    {
        Lighting::FillLightGrid(gfxContext, camera);

        if (!SSAO::DebugDraw)
        {
            ScopedTimer _prof(L"Main Render", gfxContext);
            {
                gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                gfxContext.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
                gfxContext.ClearColor(g_SceneColorBuffer);
            }
        }
    }

    if (!skipShadowMap)
    {
        if (!SSAO::DebugDraw)
        {
            pfnSetupGraphicsState();
            {
                ScopedTimer _prof2(L"Render Shadow Map", gfxContext);

                m_SunShadow.UpdateMatrix(-m_SunDirection, Vector3(0, -500.0f, 0), Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
                    (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

                g_ShadowBuffer.BeginRendering(gfxContext);
                gfxContext.SetPipelineState(m_ShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), camera.GetPosition(), kOpaque);
                gfxContext.SetPipelineState(m_CutoutShadowPSO);
                RenderObjects(gfxContext, m_SunShadow.GetViewProjMatrix(), camera.GetPosition(), kCutout);
                g_ShadowBuffer.EndRendering(gfxContext);
            }
        }
    }

    if (!skipDiffusePass)
    {
        if (!SSAO::DebugDraw)
        {
            if (SSAO::AsyncCompute)
            {
                gfxContext.Flush();
                pfnSetupGraphicsState();

                // Make the 3D queue wait for the Compute queue to finish SSAO
                g_CommandManager.GetGraphicsQueue().StallForProducer(g_CommandManager.GetComputeQueue());
            }

            {
                ScopedTimer _prof2(L"Render Color", gfxContext);

                gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);
                gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

                {
                    gfxContext.SetPipelineState(m_ModelPSO);
                    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
                    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
                    gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
                    gfxContext.SetViewportAndScissor(viewport, scissor);
                }
                RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque );

                gfxContext.SetPipelineState(m_CutoutModelPSO);
                RenderObjects( gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kCutout );
            }
        }
    }
}

void Sponza::RenderSceneRenderGraph(GraphicsContext& gfxContext, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor, bool skipDiffusePass, bool skipShadowMap)
{
    Renderer::UpdateGlobalDescriptors();
    std::uint32_t frameIndex = TemporalEffects::GetFrameIndexMod2();

    // Calculate sun direction
    float costheta = cosf(m_SunOrientation);
    float sintheta = sinf(m_SunOrientation);
    float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
    float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
    m_SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));

    if (g_renderGraph == nullptr) {
        g_renderGraph = new RenderGraph::RenderGraph();
    }
    else
    {
        g_renderGraph->Clear();
    }

    // Define render passes using lambdas that capture the needed context
    auto shadowPass = [&](ID3D12GraphicsCommandList* cmdList) {
        ScopedTimer _prof(L"Render Shadow Map", gfxContext);

        m_SunShadow.UpdateMatrix(
            -m_SunDirection,
            Vector3(0.0f, -500.0f, 0.0f),
            Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
            static_cast<std::uint32_t>(g_ShadowBuffer.GetWidth()),
            static_cast<std::uint32_t>(g_ShadowBuffer.GetHeight()),
            16);

        g_ShadowBuffer.BeginRendering(gfxContext);

        // ----- Shadow PSO

        gfxContext.SetPipelineState(m_ShadowPSO);
        RenderObjects(
            gfxContext,
            m_SunShadow.GetViewProjMatrix(),
            camera.GetPosition(),
            kOpaque);

        // ----- Cutout Shadow PSO

        gfxContext.SetPipelineState(m_CutoutShadowPSO);
        RenderObjects(
            gfxContext,
            m_SunShadow.GetViewProjMatrix(),
            camera.GetPosition(),
            kCutout
        );

        g_ShadowBuffer.EndRendering(gfxContext);

    };

    auto depthPrePass = [&](ID3D12GraphicsCommandList* cmdList) {
        ScopedTimer _prof(L"Z PrePass", gfxContext);

        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
        gfxContext.ClearDepth(g_SceneDepthBuffer);
        gfxContext.SetPipelineState(m_DepthPSO);
        gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
        gfxContext.SetViewportAndScissor(viewport, scissor);
        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kOpaque);

        gfxContext.SetPipelineState(m_CutoutDepthPSO);
        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), kCutout);
    };

    auto mainRenderPass = [&](ID3D12GraphicsCommandList* cmdList) {
        ScopedTimer _prof(L"Main Render", gfxContext);

        gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(g_SceneColorBuffer);

        gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);

        // Set up PS constants
        struct {
            Vector3 sunDirection;
            Vector3 sunLight;
            Vector3 ambientLight;
            float ShadowTexelSize[4];
            float InvTileDim[4];
            uint32_t TileCount[4];
            uint32_t FirstLightIndex[4];
            uint32_t FrameIndexMod2;
        } psConstants;

        psConstants.sunDirection        = m_SunDirection;
        psConstants.sunLight            = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
        psConstants.ambientLight        = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
        psConstants.ShadowTexelSize[0]  = 1.0f / g_ShadowBuffer.GetWidth();
        psConstants.InvTileDim[0]       = 1.0f / Lighting::LightGridDim;
        psConstants.InvTileDim[1]       = 1.0f / Lighting::LightGridDim;
        psConstants.TileCount[0]        = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
        psConstants.TileCount[1]        = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
        psConstants.FirstLightIndex[0]  = Lighting::m_FirstConeLight;
        psConstants.FirstLightIndex[1]  = Lighting::m_FirstConeShadowedLight;
        psConstants.FrameIndexMod2      = frameIndex;

        gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

        gfxContext.SetPipelineState(m_ModelPSO);
        gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
        gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
        gfxContext.SetViewportAndScissor(viewport, scissor);

        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kOpaque);
        gfxContext.SetPipelineState(m_CutoutModelPSO);
        RenderObjects(gfxContext, camera.GetViewProjMatrix(), camera.GetPosition(), Sponza::kCutout);
    };

    /////////////////////////////////////////
    // Create resources in the render graph
    /////////////////////////////////////////

    auto backBuffer = g_renderGraph->CreateResource(L"BackBuffer", {
        RenderGraph::RenderGraphResourceType::RTV,
        g_SceneColorBuffer.GetFormat(),
        g_SceneColorBuffer.GetWidth(),
        g_SceneColorBuffer.GetHeight()
    });
    backBuffer->SetResource(g_SceneColorBuffer.GetResource());

    auto depthBuffer = g_renderGraph->CreateResource(L"DepthBuffer", {
        RenderGraph::RenderGraphResourceType::DSV,
        g_SceneDepthBuffer.GetFormat(),
        g_SceneDepthBuffer.GetWidth(),
        g_SceneDepthBuffer.GetHeight()
    });
    depthBuffer->SetResource(g_SceneDepthBuffer.GetResource());

    auto normalBuffer = g_renderGraph->CreateResource(L"NormalBuffer", {
        RenderGraph::RenderGraphResourceType::RTV,
        g_SceneNormalBuffer.GetFormat(),
        g_SceneNormalBuffer.GetWidth(),
        g_SceneNormalBuffer.GetHeight()
    });
    normalBuffer->SetResource(g_SceneNormalBuffer.GetResource());

    auto shadowBuffer = g_renderGraph->CreateResource(L"ShadowBuffer", {
        RenderGraph::RenderGraphResourceType::DSV,
        g_ShadowBuffer.GetFormat(),
        g_ShadowBuffer.GetWidth(),
        g_ShadowBuffer.GetHeight()
    });
    shadowBuffer->SetResource(g_ShadowBuffer.GetResource());

    // -------- Create render passes

    auto shadowPassNode     = std::make_unique<RenderGraph::LambdaRenderPass>(L"ShadowPass", shadowPass);
    auto depthPrePassNode   = std::make_unique<RenderGraph::LambdaRenderPass>(L"DepthPrePass", depthPrePass);
    auto mainRenderPassNode = std::make_unique<RenderGraph::LambdaRenderPass>(L"MainRenderPass", mainRenderPass);

    shadowPassNode->SetMultiAdapterAllowed(true, 1);
    shadowPassNode->AddDependentAdapter(1);

    depthPrePassNode->SetMultiAdapterAllowed(true, 1);
    depthPrePassNode->AddDependentAdapter(0);

    // -------- Add render passes to graph

    std::size_t shadowPassId = g_renderGraph->AddNode(std::move(shadowPassNode));
    std::size_t depthPrePassId = g_renderGraph->AddNode(std::move(depthPrePassNode));
    std::size_t mainRenderPassId = g_renderGraph->AddNode(std::move(mainRenderPassNode));

    // -------- Add edge dependencies

    if (!skipShadowMap) {
        g_renderGraph->AddEdge(
            shadowPassId,
            depthPrePassId,
            std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
                RenderGraph::RenderGraphEdgeResourceData{
                    shadowBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
                })
        );
    }

    g_renderGraph->AddEdge(
        depthPrePassId,
        mainRenderPassId,
        std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
            RenderGraph::RenderGraphEdgeResourceData{
                depthBuffer,
                D3D12_RESOURCE_STATE_DEPTH_READ
            })
    );

    if (!SSAO::DebugDraw) {
        auto ssaoPass = [&](ID3D12GraphicsCommandList* cmdList) {
            SSAO::Render(gfxContext, camera);
            };

        auto ssaoPassNode = std::make_unique<RenderGraph::LambdaRenderPass>(L"SSAOPass", ssaoPass);
        std::size_t ssaoPassId = g_renderGraph->AddNode(std::move(ssaoPassNode));

        g_renderGraph->AddEdge(
            depthPrePassId,
            mainRenderPassId,
            std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
                RenderGraph::RenderGraphEdgeResourceData{
                    depthBuffer,
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
                })
        );

        g_renderGraph->AddEdge(
            ssaoPassId,
            mainRenderPassId,
            std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
                RenderGraph::RenderGraphEdgeResourceData{
                    nullptr,
                    D3D12_RESOURCE_STATE_COMMON
                })
        );
    }

    if (!skipDiffusePass) {
        auto lightGridPass = [&](ID3D12GraphicsCommandList* cmdList) {
            Lighting::FillLightGrid(gfxContext, camera);
            };

        auto lightGridPassNode = std::make_unique<RenderGraph::LambdaRenderPass>(L"LightGridPass", lightGridPass);
        std::size_t lightGridPassId = g_renderGraph->AddNode(std::move(lightGridPassNode));

        g_renderGraph->AddEdge(
            depthPrePassId,
            lightGridPassId,
            std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
                RenderGraph::RenderGraphEdgeResourceData{
                    depthBuffer,
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
                })
        );

        g_renderGraph->AddEdge(
            lightGridPassId,
            mainRenderPassId,
            std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
                RenderGraph::RenderGraphEdgeResourceData{
                    nullptr,
                    D3D12_RESOURCE_STATE_COMMON
                })
        );
    }

    g_renderGraph->Compile();
    gfxContext.SetRootSignature(Renderer::m_RootSig);
    gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Renderer::s_TextureHeap.GetHeapPointer());
    gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
    gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
    g_renderGraph->Execute(gfxContext);
}

void Sponza::RenderSceneRenderGraphStoraged(GraphicsContext& gfxContext, const Math::Camera& camera, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor, bool skipDiffusePass, bool skipShadowMap)
{
    m_Camera = camera;
    m_Viewport = viewport;
    m_Scissor = scissor;
    Renderer::UpdateGlobalDescriptors();
    m_FrameIndex = TemporalEffects::GetFrameIndexMod2();

    // Calculate sun direction
    float costheta = cosf(m_SunOrientation);
    float sintheta = sinf(m_SunOrientation);
    float cosphi = cosf(m_SunInclination * 3.14159f * 0.5f);
    float sinphi = sinf(m_SunInclination * 3.14159f * 0.5f);
    m_SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));

    g_renderGraphStoraged->Clear();

    std::size_t particleUpdatePassId = g_renderGraphStoraged->AddNode(L"ParticleUpdatePass");
    std::size_t lightShadowPassId = g_renderGraphStoraged->AddNode(L"LightShadowPass");
    std::size_t shadowPassId = g_renderGraphStoraged->AddNode(L"ShadowPass");
    std::size_t depthPrePassId = g_renderGraphStoraged->AddNode(L"DepthPrePass");
    std::size_t mainRenderPassId = g_renderGraphStoraged->AddNode(L"MainRenderPass");
    std::size_t velocityPassId = g_renderGraphStoraged->AddNode(L"VelocityGatherPass");
    std::size_t taaResolvePassId = g_renderGraphStoraged->AddNode(L"TAAResolvePass");
    std::size_t particleRenderPassId = g_renderGraphStoraged->AddNode(L"ParticleRenderPass");

    // -------- Add edge dependencies

    // TODO: some of the nodes just use dummy resources for order control
    // figure out actual dependencies later. Especially for particles

    g_renderGraphStoraged->AddEdge(
        particleUpdatePassId,
        particleRenderPassId,
        L"Dummy"
    );

    g_renderGraphStoraged->AddEdge(
        lightShadowPassId,
        depthPrePassId,
        L"Dummy"
    );

    if (!skipShadowMap) {
        g_renderGraphStoraged->AddEdge(
            lightShadowPassId,
            shadowPassId,
            L"Dummy"
        );

        g_renderGraphStoraged->AddEdge(
            shadowPassId,
            mainRenderPassId,
            L"Dummy"
        );

        g_renderGraphStoraged->AddEdge(
            depthPrePassId,
            shadowPassId,
            L"ShadowPixel"
        );
    }

    g_renderGraphStoraged->AddEdge(
        depthPrePassId,
        mainRenderPassId,
        L"DepthRead"
    );

    if (!SSAO::DebugDraw) {
        std::size_t ssaoPassId = g_renderGraphStoraged->AddNode(L"SSAOPass");

        g_renderGraphStoraged->AddEdge(
            depthPrePassId,
            ssaoPassId,
            L"DepthNonPixel"
        );

        g_renderGraphStoraged->AddEdge(
            ssaoPassId,
            mainRenderPassId,
            L"Dummy"
        );
    }

    if (!skipDiffusePass) {
        std::size_t lightGridPassId = g_renderGraphStoraged->AddNode(L"LightGridPass");

        g_renderGraphStoraged->AddEdge(
            depthPrePassId,
            lightGridPassId,
            L"DepthNonPixel"
        );

        g_renderGraphStoraged->AddEdge(
            lightGridPassId,
            mainRenderPassId,
            L"Dummy"
        );
    }

    g_renderGraphStoraged->AddEdge(
        mainRenderPassId,
        velocityPassId,
        L"ColorNonPixel"
    );

    g_renderGraphStoraged->AddEdge(
        velocityPassId,
        taaResolvePassId,
        L"VelocityNonPixel"
    );

    g_renderGraphStoraged->AddEdge(
        taaResolvePassId,
        particleRenderPassId,
        L"ColorRenderTarget"
    );

    // DoF and motion blur implementations are currently incompatible
    // DoF has priority when enabled
    if (DepthOfField::Enable) {
        std::size_t depthOfFieldPassId = g_renderGraphStoraged->AddNode(L"DepthOfFieldPass");
        g_renderGraphStoraged->AddEdge(
            particleRenderPassId,
            depthOfFieldPassId,
            L"ColorNonPixel"
        );
    } else { // Enable Motion Blur instead
        std::size_t motionBlurPassId = g_renderGraphStoraged->AddNode(L"MotionBlurPass");
        g_renderGraphStoraged->AddEdge(
            particleRenderPassId,
            motionBlurPassId,
            L"ColorNonPixel"
        );
        g_renderGraphStoraged->AddEdge(
            velocityPassId,
            motionBlurPassId,
            L"VelocityNonPixel"
        );
    }

    g_renderGraphStoraged->Compile();
    g_renderGraphStoraged->Execute(gfxContext);
}

void Sponza::RenderGraphStartup()
{
    g_renderGraph = new RenderGraph::RenderGraph();
    g_renderGraphStoraged = new RenderGraph::RenderGraphStoraged();
    // Create resources once
    // TODO: find a way to represent particle resources
    auto backBuffer = g_renderGraphStoraged->CreateResource(L"BackBuffer", {
        RenderGraph::RenderGraphResourceType::RTV,
        g_SceneColorBuffer.GetFormat(),
        g_SceneColorBuffer.GetWidth(),
        g_SceneColorBuffer.GetHeight()
    });
    backBuffer->SetResource(g_SceneColorBuffer.GetResource());

    auto depthBuffer = g_renderGraphStoraged->CreateResource(L"DepthBuffer", {
        RenderGraph::RenderGraphResourceType::DSV,
        g_SceneDepthBuffer.GetFormat(),
        g_SceneDepthBuffer.GetWidth(),
        g_SceneDepthBuffer.GetHeight()
    });
    depthBuffer->SetResource(g_SceneDepthBuffer.GetResource());

    auto normalBuffer = g_renderGraphStoraged->CreateResource(L"NormalBuffer", {
        RenderGraph::RenderGraphResourceType::RTV,
        g_SceneNormalBuffer.GetFormat(),
        g_SceneNormalBuffer.GetWidth(),
        g_SceneNormalBuffer.GetHeight()
    });
    normalBuffer->SetResource(g_SceneNormalBuffer.GetResource());

    auto shadowBuffer = g_renderGraphStoraged->CreateResource(L"ShadowBuffer", {
        RenderGraph::RenderGraphResourceType::DSV,
        g_ShadowBuffer.GetFormat(),
        g_ShadowBuffer.GetWidth(),
        g_ShadowBuffer.GetHeight()
    });
    shadowBuffer->SetResource(g_ShadowBuffer.GetResource());

    auto velocityBuffer = g_renderGraphStoraged->CreateResource(L"VelocityBuffer", {
        RenderGraph::RenderGraphResourceType::RTV,
        g_VelocityBuffer.GetFormat(),
        g_VelocityBuffer.GetWidth(),
        g_VelocityBuffer.GetHeight()
    });
    velocityBuffer->SetResource(g_VelocityBuffer.GetResource());
    // Create storaged nodes to use/reuse later on
    auto shadowPassNode         = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"ShadowPass", &Sponza::ShadowPass);
    auto depthPrePassNode       = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"DepthPrePass", &Sponza::DepthPrePass);
    auto mainRenderPassNode     = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"MainRenderPass", &Sponza::MainRenderPass);
    auto ssaoPassNode           = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"SSAOPass", &Sponza::SSAOPass);
    auto lightGridPassNode      = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"LightGridPass", &Sponza::LightGridPass);
    auto lightShadowPassNode    = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"LightShadowPass", &Sponza::LightShadowPass);
    auto taaResolvePassNode     = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"TAAResolvePass", &Sponza::ResolveTAA);
    auto velocityPassNode       = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"VelocityGatherPass", &Sponza::GenerateCameraVelocityBuffer);
    auto motionBlurPassNode     = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"MotionBlurPass", &Sponza::RenderBlur);
    auto depthOfFieldPassNode   = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"DepthOfFieldPass", &Sponza::RenderDOF);
    auto particleRenderPassNode = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"ParticleRenderPass", &Sponza::RenderParticles);
    auto particleUpdatePassNode = std::make_unique<RenderGraph::LambdaContextRenderPass>(L"ParticleUpdatePass", &Sponza::RecalculateParticles);
    g_renderGraphStoraged->StoreNode(std::move(particleUpdatePassNode), L"ParticleUpdatePass");
    g_renderGraphStoraged->StoreNode(std::move(particleRenderPassNode), L"ParticleRenderPass");
    g_renderGraphStoraged->StoreNode(std::move(depthOfFieldPassNode), L"DepthOfFieldPass");
    g_renderGraphStoraged->StoreNode(std::move(motionBlurPassNode), L"MotionBlurPass");
    g_renderGraphStoraged->StoreNode(std::move(velocityPassNode), L"VelocityGatherPass");
    g_renderGraphStoraged->StoreNode(std::move(taaResolvePassNode), L"TAAResolvePass");
    g_renderGraphStoraged->StoreNode(std::move(lightShadowPassNode), L"LightShadowPass");
    g_renderGraphStoraged->StoreNode(std::move(lightGridPassNode), L"LightGridPass");
    g_renderGraphStoraged->StoreNode(std::move(ssaoPassNode), L"SSAOPass");
    g_renderGraphStoraged->StoreNode(std::move(shadowPassNode), L"ShadowPass");
    g_renderGraphStoraged->StoreNode(std::move(depthPrePassNode), L"DepthPrePass");
    g_renderGraphStoraged->StoreNode(std::move(mainRenderPassNode), L"MainRenderPass");
    // Create edge transition data
    auto shadowPixelData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            shadowBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        });
    auto velocityNonPixelData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            velocityBuffer,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        });
    auto velocityUnorderedData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            velocityBuffer,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        });
    auto velocityPixelData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            velocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        });
    auto depthNonPixelData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            depthBuffer,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        });
    auto colorNonPixelData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            backBuffer,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        });
    auto colorUnorderedData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            backBuffer,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        });
    auto colorRenderTargetData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        });
    auto depthReadData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            depthBuffer,
            D3D12_RESOURCE_STATE_DEPTH_READ
        });
    auto dummyData = std::make_unique<RenderGraph::RenderGraphEdgeResourceData>(
        RenderGraph::RenderGraphEdgeResourceData{
            nullptr,
            D3D12_RESOURCE_STATE_COMMON
        });
    g_renderGraphStoraged->StoreEdge(std::move(shadowPixelData), L"ShadowPixel");
    g_renderGraphStoraged->StoreEdge(std::move(depthNonPixelData), L"DepthNonPixel");
    g_renderGraphStoraged->StoreEdge(std::move(velocityNonPixelData), L"VelocityNonPixel");
    g_renderGraphStoraged->StoreEdge(std::move(velocityUnorderedData), L"VelocityUnordered");
    g_renderGraphStoraged->StoreEdge(std::move(velocityPixelData), L"VelocityPixel");
    g_renderGraphStoraged->StoreEdge(std::move(colorNonPixelData), L"ColorNonPixel");
    g_renderGraphStoraged->StoreEdge(std::move(colorUnorderedData), L"ColorUnordered");
    g_renderGraphStoraged->StoreEdge(std::move(colorNonPixelData), L"ColorRenderTarget");
    g_renderGraphStoraged->StoreEdge(std::move(depthReadData), L"DepthRead");
    g_renderGraphStoraged->StoreEdge(std::move(dummyData), L"Dummy");
}

void Sponza::ShadowPass(CommandContext& ctx)
{
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    ScopedTimer _prof(L"Render Shadow Map", gfxContext);
    SetupGraphicsState(gfxContext);

    m_SunShadow.UpdateMatrix(
        -m_SunDirection,
        Vector3(0.0f, -500.0f, 0.0f),
        Vector3(ShadowDimX, ShadowDimY, ShadowDimZ),
        static_cast<std::uint32_t>(g_ShadowBuffer.GetWidth()),
        static_cast<std::uint32_t>(g_ShadowBuffer.GetHeight()),
        16);

    g_ShadowBuffer.BeginRendering(gfxContext);

    // ----- Shadow PSO

    gfxContext.SetPipelineState(m_ShadowPSO);
    RenderObjects(
        gfxContext,
        m_SunShadow.GetViewProjMatrix(),
        m_Camera.GetPosition(),
        kOpaque);

    // ----- Cutout Shadow PSO

    gfxContext.SetPipelineState(m_CutoutShadowPSO);
    RenderObjects(
        gfxContext,
        m_SunShadow.GetViewProjMatrix(),
        m_Camera.GetPosition(),
        kCutout
    );

    g_ShadowBuffer.EndRendering(gfxContext);
}

void Sponza::DepthPrePass(CommandContext& ctx)
{
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    ScopedTimer _prof(L"Z PrePass", gfxContext);

    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    gfxContext.ClearDepth(g_SceneDepthBuffer);
    gfxContext.SetPipelineState(m_DepthPSO);
    gfxContext.SetDepthStencilTarget(g_SceneDepthBuffer.GetDSV());
    gfxContext.SetViewportAndScissor(m_Viewport, m_Scissor);
    RenderObjects(gfxContext, m_Camera.GetViewProjMatrix(), m_Camera.GetPosition(), kOpaque);

    gfxContext.SetPipelineState(m_CutoutDepthPSO);
    RenderObjects(gfxContext, m_Camera.GetViewProjMatrix(), m_Camera.GetPosition(), kCutout);
}

void Sponza::MainRenderPass(CommandContext& ctx)
{
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    ScopedTimer _prof(L"Main Render", gfxContext);

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.TransitionResource(g_SceneNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.ClearColor(g_SceneColorBuffer);

    gfxContext.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    gfxContext.SetDescriptorTable(Renderer::kCommonSRVs, Renderer::m_CommonTextures);

    // Set up PS constants
    struct {
        Vector3 sunDirection;
        Vector3 sunLight;
        Vector3 ambientLight;
        float ShadowTexelSize[4];
        float InvTileDim[4];
        uint32_t TileCount[4];
        uint32_t FirstLightIndex[4];
        uint32_t FrameIndexMod2;
    } psConstants;

    psConstants.sunDirection        = m_SunDirection;
    psConstants.sunLight            = Vector3(1.0f, 1.0f, 1.0f) * m_SunLightIntensity;
    psConstants.ambientLight        = Vector3(1.0f, 1.0f, 1.0f) * m_AmbientIntensity;
    psConstants.ShadowTexelSize[0]  = 1.0f / g_ShadowBuffer.GetWidth();
    psConstants.InvTileDim[0]       = 1.0f / Lighting::LightGridDim;
    psConstants.InvTileDim[1]       = 1.0f / Lighting::LightGridDim;
    psConstants.TileCount[0]        = Math::DivideByMultiple(g_SceneColorBuffer.GetWidth(), Lighting::LightGridDim);
    psConstants.TileCount[1]        = Math::DivideByMultiple(g_SceneColorBuffer.GetHeight(), Lighting::LightGridDim);
    psConstants.FirstLightIndex[0]  = Lighting::m_FirstConeLight;
    psConstants.FirstLightIndex[1]  = Lighting::m_FirstConeShadowedLight;
    psConstants.FrameIndexMod2      = m_FrameIndex;

    gfxContext.SetDynamicConstantBufferView(Renderer::kMaterialConstants, sizeof(psConstants), &psConstants);

    gfxContext.SetPipelineState(m_ModelPSO);
    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[]{ g_SceneColorBuffer.GetRTV(), g_SceneNormalBuffer.GetRTV() };
    gfxContext.SetRenderTargets(ARRAYSIZE(rtvs), rtvs, g_SceneDepthBuffer.GetDSV_DepthReadOnly());
    gfxContext.SetViewportAndScissor(m_Viewport, m_Scissor);

    RenderObjects(gfxContext, m_Camera.GetViewProjMatrix(), m_Camera.GetPosition(), Sponza::kOpaque);
    gfxContext.SetPipelineState(m_CutoutModelPSO);
    RenderObjects(gfxContext, m_Camera.GetViewProjMatrix(), m_Camera.GetPosition(), Sponza::kCutout);
}

void Sponza::SSAOPass(CommandContext& ctx) {
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    SSAO::Render(gfxContext, m_Camera);
};

void Sponza::LightGridPass(CommandContext& ctx) {
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    Lighting::FillLightGrid(gfxContext, m_Camera);
};

void Sponza::LightShadowPass(CommandContext& ctx) {
    GraphicsContext& gfxContext = ctx.GetGraphicsContext();
    SetupGraphicsState(gfxContext);
    RenderLightShadows(gfxContext, m_Camera);
}

void Sponza::SetupGraphicsState(GraphicsContext& gfxContext) {
    gfxContext.SetRootSignature(Renderer::m_RootSig);
    gfxContext.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Renderer::s_TextureHeap.GetHeapPointer());
    gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfxContext.SetIndexBuffer(m_Model.GetIndexBuffer());
    gfxContext.SetVertexBuffer(0, m_Model.GetVertexBuffer());
}

void Sponza::RecalculateParticles(CommandContext& ctx) {
    ParticleEffectManager::Update(ctx.GetComputeContext(), Graphics::GetFrameTime());
}

void Sponza::GenerateCameraVelocityBuffer(CommandContext& ctx) {
    // Some systems generate a per-pixel velocity buffer to better track dynamic and skinned meshes.  Everything
    // is static in our scene, so we generate velocity from camera motion and the depth buffer.  A velocity buffer
    // is necessary for all temporal effects (and motion blur).
    MotionBlur::GenerateCameraVelocityBuffer(ctx.GetGraphicsContext(), m_Camera, true);
}

void Sponza::ResolveTAA(CommandContext& ctx) {
    TemporalEffects::ResolveImage(ctx.GetGraphicsContext());
}

void Sponza::RenderParticles(CommandContext& ctx) {
    ParticleEffectManager::Render(ctx.GetGraphicsContext(), m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer,  g_LinearDepth[m_FrameIndex]);
}

void Sponza::RenderDOF(CommandContext& ctx) {
    DepthOfField::Render(ctx.GetGraphicsContext(), m_Camera.GetNearClip(), m_Camera.GetFarClip());
}

void Sponza::RenderBlur(CommandContext& ctx) {
    MotionBlur::RenderObjectBlur(ctx.GetGraphicsContext(), g_VelocityBuffer);
}
