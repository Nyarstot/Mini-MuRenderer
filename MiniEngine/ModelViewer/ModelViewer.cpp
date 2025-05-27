#include "GameCore.h"
#include "CameraController.h"
#include "BufferManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"
#include "DepthOfField.h"
#include "PostEffects.h"
#include "SSAO.h"
#include "FXAA.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "ParticleEffectManager.h"
#include "ParticleEffects.h"
#include "GameInput.h"
#include "SponzaRenderer.h"
#include "glTF.h"
#include "Renderer.h"
#include "Model.h"
#include "ModelLoader.h"
#include "ShadowCamera.h"
#include "Display.h"

#include <direct.h> // for _getcwd() to check data root path

#include "../Core/MultiGPU/DeviceBenchmark.h"
#include "../Core/MultiGPU/SharedResource.h"
#include "../Core/RenderGraph/RenderGraphInclude.h"

using namespace GameCore;
using namespace Math;
using namespace Graphics;
using namespace std;
using Renderer::MeshSorter;


class MuExample : public GameCore::IGameApp
{
private:
    std::unique_ptr<CameraController> m_CameraController;
    Camera m_Camera;

    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    ModelInstance m_ModelInstance;
    ShadowCamera m_SunShadowCamera;
    RenderGraph::RenderGraph* m_renderGraph;

    bool m_AllowEMASupport = false;
    MultiGPU::DeviceBenchmarkProvider m_PrimaryBenchmarkProvider;
    MultiGPU::DeviceBenchmarkProvider m_SecondaryBenchmarkProvider;

private:
    void RenderGraphStartup(void);

public:
    MuExample() = default;
    ~MuExample() = default;

    virtual void Startup(void) override;
    virtual void Cleanup(void) override;

    virtual void Update(float deltaT) override;
    virtual void RenderScene(void) override;

public:
    void MeshRenderPass(CommandContext& ctx);



};

CREATE_APPLICATION(MuExample)

ExpVar g_SunLightIntensity("Viewer/Lighting/Sun Light Intensity", 4.0f, 0.0f, 16.0f, 0.1f);
NumVar g_SunOrientation("Viewer/Lighting/Sun Orientation", -0.5f, -100.0f, 100.0f, 0.1f);
NumVar g_SunInclination("Viewer/Lighting/Sun Inclination", 0.75f, 0.0f, 1.0f, 0.01f);

void ChangeIBLSet(EngineVar::ActionType);
void ChangeIBLBias(EngineVar::ActionType);

DynamicEnumVar g_IBLSet("Viewer/Lighting/Environment", ChangeIBLSet);
std::vector<std::pair<TextureRef, TextureRef>> g_IBLTextures;
NumVar g_IBLBias("Viewer/Lighting/Gloss Reduction", 2.0f, 0.0f, 10.0f, 1.0f, ChangeIBLBias);

////////////////////
// IBL
///////////////////

void ChangeIBLSet(EngineVar::ActionType)
{
    int setIdx = g_IBLSet - 1;
    if (setIdx < 0)
    {
        Renderer::SetIBLTextures(nullptr, nullptr);
    }
    else
    {
        auto texturePair = g_IBLTextures[setIdx];
        Renderer::SetIBLTextures(texturePair.first, texturePair.second);
    }
}

void ChangeIBLBias(EngineVar::ActionType)
{
    Renderer::SetIBLBias(g_IBLBias);
}

void LoadIBLTextures()
{
    char CWD[256];
    _getcwd(CWD, 256);

    Utility::Printf("Loading IBL environment maps\n");

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(L"Textures/*_diffuseIBL.dds", &ffd);

    g_IBLSet.AddEnum(L"None");

    if (hFind != INVALID_HANDLE_VALUE) do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::wstring diffuseFile = ffd.cFileName;
        std::wstring baseFile = diffuseFile;
        baseFile.resize(baseFile.rfind(L"_diffuseIBL.dds"));
        std::wstring specularFile = baseFile + L"_specularIBL.dds";

        TextureRef diffuseTex = TextureManager::LoadDDSFromFile(L"Textures/" + diffuseFile);
        if (diffuseTex.IsValid())
        {
            TextureRef specularTex = TextureManager::LoadDDSFromFile(L"Textures/" + specularFile);
            if (specularTex.IsValid())
            {
                g_IBLSet.AddEnum(baseFile);
                g_IBLTextures.push_back(std::make_pair(diffuseTex, specularTex));
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    Utility::Printf("Found %u IBL environment map sets\n", g_IBLTextures.size());

    if (g_IBLTextures.size() > 0)
        g_IBLSet.Increment();
}

////////////////////
// GENERAL
///////////////////

void MuExample::Startup(void)
{
    MotionBlur::Enable              = true;
    TemporalEffects::EnableTAA      = true;
    FXAA::Enable                    = false;
    PostEffects::EnableHDR          = true;
    PostEffects::EnableAdaptation   = true;
    SSAO::Enable                    = true;

    Renderer::Initialize();
    LoadIBLTextures();

    std::wstring gltfFileName;
    bool forceRebuild = false;

    std::uint32_t rebuildValue;
    if (CommandLineArgs::GetInteger(L"rebuild", rebuildValue))
        forceRebuild = rebuildValue != 0;

    m_ModelInstance = Renderer::LoadModel(L"Sponza/PBR/sponza2.gltf", forceRebuild);
    m_ModelInstance.Resize(100.0f * m_ModelInstance.GetRadius());
    OrientedBox obb = m_ModelInstance.GetBoundingBox();

    float modelRadius = Length(obb.GetDimensions()) * 0.5f;
    const Vector3 eye = obb.GetCenter() + Vector3(modelRadius * 0.5f, 0.0f, 0.0f);
    m_Camera.SetEyeAtUp(eye, Vector3(kZero), Vector3(kYUnitVector));

    // Setup PSOs

    DXGI_FORMAT ColorFormat     = g_SceneColorBuffer.GetFormat();
    DXGI_FORMAT NormalFormat    = g_SceneNormalBuffer.GetFormat();
    DXGI_FORMAT DepthFormat     = g_SceneDepthBuffer.GetFormat();

    D3D12_INPUT_ELEMENT_DESC vertElem[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    m_Camera.SetZRange(1.0f, 10000.0f);
    if (gltfFileName.size() == 0)
        m_CameraController.reset(new FlyingFPSCamera(m_Camera, Vector3(kYUnitVector)));
    else
        m_CameraController.reset(new OrbitCamera(m_Camera, m_ModelInstance.GetBoundingSphere(), Vector3(kYUnitVector)));

    ParticleEffects::InitFromJSON(L"Sponza/particles.json");

    // Init render graph

    if (!m_renderGraph) {
        m_renderGraph = new RenderGraph::RenderGraph(L"MuRenderer Render Graph");
        RenderGraphStartup();
    }

    // Perform MultiGPU benchmarks

    if (m_AllowEMASupport) {
        m_PrimaryBenchmarkProvider.Initialize(Graphics::g_Device);
        m_SecondaryBenchmarkProvider.Initialize(Graphics::g_SecondaryDevice);

        m_PrimaryBenchmarkProvider.PerformBenchmark(1);
        m_SecondaryBenchmarkProvider.PerformBenchmark(1);
    }
}

void MuExample::RenderGraphStartup(void)
{
    Utility::Printf(L"Register RenderGraph with name: %s\n", m_renderGraph->GetName());

    auto sceneDepthBufferRes    = m_renderGraph->RegisterExternalResource<DepthBuffer>(L"SceneDepthBuffer", &g_SceneDepthBuffer);
    auto sceneColorBufferRes    = m_renderGraph->RegisterExternalResource<ColorBuffer>(L"SceneColorBuffer", &g_SceneColorBuffer);
    auto ssaoFullScreenRes      = m_renderGraph->RegisterExternalResource<ColorBuffer>(L"SSAOFullScreen", &g_SSAOFullScreen);
    auto shadowBufferRes        = m_renderGraph->RegisterExternalResource<ShadowBuffer>(L"ShadowBuffer", &g_ShadowBuffer);
}

namespace Graphics
{
    extern EnumVar DebugZoom;
}

void MuExample::Update(float deltaT)
{
    ScopedTimer _prof(L"Update State");
    m_CameraController->Update(deltaT);

    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Update");
    m_ModelInstance.Update(gfxContext, deltaT);
    gfxContext.Finish();

    // We use viewport offsets to jitter sample positions from frame to frame (for TAA.)
    // D3D has a design quirk with fractional offsets such that the implicit scissor
    // region of a viewport is floor(TopLeftXY) and floor(TopLeftXY + WidthHeight), so
    // having a negative fractional top left, e.g. (-0.25, -0.25) would also shift the
    // BottomRight corner up by a whole integer.  One solution is to pad your viewport
    // dimensions with an extra pixel.  My solution is to only use positive fractional offsets,
    // but that means that the average sample position is +0.5, which I use when I disable
    // temporal AA.
    TemporalEffects::GetJitterOffset(m_MainViewport.TopLeftX, m_MainViewport.TopLeftY);

    m_MainViewport.Width    = (float)g_SceneColorBuffer.GetWidth();
    m_MainViewport.Height   = (float)g_SceneColorBuffer.GetHeight();
    m_MainViewport.MinDepth = 0.0f;
    m_MainViewport.MaxDepth = 1.0f;

    m_MainScissor.left      = 0;
    m_MainScissor.top       = 0;
    m_MainScissor.right     = (LONG)g_SceneColorBuffer.GetWidth();
    m_MainScissor.bottom    = (LONG)g_SceneColorBuffer.GetHeight();
}

void MuExample::RenderScene(void)
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Render Scene");

    std::uint32_t frameIndex        = TemporalEffects::GetFrameIndexMod2();
    const D3D12_VIEWPORT& viewport  = m_MainViewport;
    const D3D12_RECT& scissor       = m_MainScissor;

    // Update global constants

    float costheta  = cosf(g_SunOrientation);
    float sintheta  = sinf(g_SunOrientation);
    float cosphi    = cosf(g_SunInclination * 3.14159f * 0.5f);
    float sinphi    = sinf(g_SunInclination * 3.14159f * 0.5f);

    Vector3 SunDirection = Normalize(Vector3(costheta * cosphi, sinphi, sintheta * cosphi));
    Vector3 ShadowBounds = Vector3(m_ModelInstance.GetRadius());
    m_SunShadowCamera.UpdateMatrix(-SunDirection, Vector3(0, -500.0f, 0), Vector3(5000, 3000, 3000),
        (uint32_t)g_ShadowBuffer.GetWidth(), (uint32_t)g_ShadowBuffer.GetHeight(), 16);

    GlobalConstants globals;
    globals.ViewProjMatrix  = m_Camera.GetViewProjMatrix();
    globals.SunShadowMatrix = m_SunShadowCamera.GetShadowMatrix();
    globals.CameraPos       = m_Camera.GetPosition();
    globals.SunDirection    = SunDirection;
    globals.SunIntensity    = Vector3(Scalar(g_SunLightIntensity));

    // Begin rendering depth
    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    gfxContext.ClearDepth(g_SceneDepthBuffer);

    MeshSorter sorter(MeshSorter::kDefault);
    sorter.SetCamera(m_Camera);
    sorter.SetViewport(viewport);
    sorter.SetScissor(scissor);
    sorter.SetDepthStencilTarget(g_SceneDepthBuffer);
    sorter.AddRenderTarget(g_SceneColorBuffer);

    m_ModelInstance.Render(sorter);
    sorter.Sort();

    auto sceneDepthBufferRes = m_renderGraph->GetRegisteredResourceEntry(L"SceneDepthBuffer");
    auto sceneColorBufferRes = m_renderGraph->GetRegisteredResourceEntry(L"SceneColorBuffer");
    auto ssaoFullScreenRes = m_renderGraph->GetRegisteredResourceEntry(L"SSAOFullScreen");
    auto shadowBufferRes = m_renderGraph->GetRegisteredResourceEntry(L"ShadowBuffer");

    auto depthPrePass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"DepthPrePass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes, sceneColorBufferRes }),
        WriteToSpan({ &sceneDepthBufferRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"Depth Pre-Pass", ctx);
            sorter.RenderMeshes(MeshSorter::kZPass, ctx, globals);
        }
    );

    auto SSAOPass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"SSAOPass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes }),
        WriteToSpan({ &ssaoFullScreenRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"SSAO Pass", ctx);
            SSAO::Render(ctx, m_Camera);
        }
    );

    auto sunShadowPass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"SunShadowPass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes }),
        WriteToSpan({&sceneDepthBufferRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"Sun Shadow Map", ctx);
            MeshSorter shadowSorter(MeshSorter::kShadows);
            shadowSorter.SetCamera(m_SunShadowCamera);
            shadowSorter.SetDepthStencilTarget(g_ShadowBuffer);
            m_ModelInstance.Render(shadowSorter);
            shadowSorter.Sort();
            shadowSorter.RenderMeshes(MeshSorter::kZPass, ctx, globals);
            ctx.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
            ctx.ClearColor(g_SceneColorBuffer);
        }
    );

    auto colorPass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"ColorPass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes }),
        WriteToSpan({&sceneDepthBufferRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"Render Color", ctx);
            ctx.TransitionResource(g_SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            ctx.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
            ctx.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV_DepthReadOnly());
            ctx.SetViewportAndScissor(viewport, scissor);
            sorter.RenderMeshes(MeshSorter::kOpaque, ctx, globals);
        }
    );

    auto motionBlurPass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"MotionBlurPass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes }),
        WriteToSpan({ &sceneDepthBufferRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"MotionBlur Pass", ctx);
            MotionBlur::GenerateCameraVelocityBuffer(ctx, m_Camera, true);
        }
    );

    auto taaPass = std::make_unique<RenderGraph::LambdaContextRenderPass>(
        L"TAAPass", m_renderGraph,
        ReadFromSpan({ sceneDepthBufferRes }),
        WriteToSpan({ &sceneDepthBufferRes }),
        [&](GraphicsContext& ctx) {
            ScopedTimer _prof(L"TAA Pass", ctx);
            TemporalEffects::ResolveImage(ctx);

            ParticleEffectManager::Update(ctx.GetComputeContext(), Graphics::GetFrameTime());
            ParticleEffectManager::Render(ctx, m_Camera, g_SceneColorBuffer, g_SceneDepthBuffer, g_LinearDepth[frameIndex]);
        }
    );

    std::size_t depthPrePassId = m_renderGraph->AddNode(std::move(depthPrePass));
    std::size_t ssaoPassId = m_renderGraph->AddNode(std::move(SSAOPass));
    std::size_t sunShadowPassId = m_renderGraph->AddNode(std::move(sunShadowPass));
    std::size_t colorPassId = m_renderGraph->AddNode(std::move(colorPass));
    std::size_t motionBlurPassId = m_renderGraph->AddNode(std::move(motionBlurPass));
    std::size_t taaPassId = m_renderGraph->AddNode(std::move(taaPass));

    m_renderGraph->AddEdge(
        depthPrePassId,
        ssaoPassId,
        nullptr
    );

    m_renderGraph->AddEdge(
        ssaoPassId,
        sunShadowPassId,
        nullptr
    );

    m_renderGraph->AddEdge(
        sunShadowPassId,
        colorPassId,
        nullptr
    );

    m_renderGraph->AddEdge(
        colorPassId,
        motionBlurPassId,
        nullptr
    );

    m_renderGraph->AddEdge(
        motionBlurPassId,
        taaPassId,
        nullptr
    );

    m_renderGraph->Compile();
    m_renderGraph->Execute(gfxContext);
    m_renderGraph->Clear();

    //MotionBlur::GenerateCameraVelocityBuffer(gfxContext, m_Camera, true);
    //TemporalEffects::ResolveImage(gfxContext);

}

void MuExample::Cleanup(void)
{
    m_ModelInstance = nullptr;
    g_IBLTextures.clear();

    Renderer::Shutdown();
}