#include "pch.h"
#include "ImGuiRenderer.h"

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
#include "GameInput.h"
#include "SponzaRenderer.h"
#include "glTF.h"
#include "Renderer.h"
#include "Model.h"
#include "ModelLoader.h"
#include "ShadowCamera.h"
#include "Display.h"
#include "Renderer.h"


namespace GameCore
{
    extern HWND g_hWnd;
}

namespace Graphics
{
    extern ID3D12Device* g_Device;
}

namespace ImGuiRenderer
{
    void Initialize(void)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();
        DescriptorHandle guiFontHeap = Renderer::s_TextureHeap.Alloc(1);

        ImGui_ImplWin32_Init(GameCore::g_hWnd);
        ImGui_ImplDX12_Init(
            Graphics::g_Device,
            3, // Number of frames in flight
            Graphics::g_OverlayBuffer.GetFormat(),
            Renderer::s_TextureHeap.GetHeapPointer(),
            D3D12_CPU_DESCRIPTOR_HANDLE(guiFontHeap),
            D3D12_GPU_DESCRIPTOR_HANDLE(guiFontHeap)
        );
    }

    void Shutdown(void)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}