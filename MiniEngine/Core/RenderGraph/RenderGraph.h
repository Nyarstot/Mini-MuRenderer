#pragma once

#include "../Core/GpuBuffer.h"
#include "../Core/VectorMath.h"
#include "../Core/Camera.h"
#include "../Core/CommandContext.h"
#include "../Core/UploadBuffer.h"
#include "../Core/TextureManager.h"

#include "RenderGraph/Base/BaseGraph.h"
#include "RenderGraph/RenderPass.h"
#include "RenderGraph/RenderGraphRegistry.h"
#include "RenderGraph/RenderGraphResource.h"
#include "RenderGraph/RenderGraphAllocator.h"


namespace RenderGraph
{
    class RGSceneState final : NonCopyable
    {
    private:
        const Math::BaseCamera* m_camera;
        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissor;
        std::uint32_t m_numRTVs;
        ColorBuffer* m_RTV[8];
        DepthBuffer* m_DSV;

    public:
        RGSceneState() = default;
        ~RGSceneState() = default;

        void SetCamera(const Math::BaseCamera* camera) { m_camera = camera; }
        void SetViewport(const D3D12_VIEWPORT& viewport) { m_viewport = viewport; }
        void SetScissor(const D3D12_RECT& scissor) { m_scissor = scissor; }
        void SetDepthStencilTarget(DepthBuffer& DSV) { m_DSV = &DSV; }

        void AddRenderTarget(ColorBuffer& RTV) {
            ASSERT(m_numRTVs < 8);
            m_RTV[m_numRTVs++] = &RTV;
        }


        const Math::Frustum& GetWorldFrustum() const { return m_camera->GetWorldSpaceFrustum(); }
        const Math::Frustum& GetViewFrustum() const { return m_camera->GetViewSpaceFrustum(); }
        const Math::Matrix4& GetViewMatrix() const { return m_camera->GetViewMatrix(); }
    };

    struct SceneStateInfo
    {
        const Math::BaseCamera& camera;
        D3D12_VIEWPORT viewport;
        D3D12_RECT scissor;
        DepthBuffer& DSV;
    };

    class RenderGraph final : public BaseGraph<RenderPass, RenderGraphEdgeResourceData>
    {
    private:
        std::wstring m_title;
        std::vector<RenderPass*> m_renderPasses;

        RGSceneState m_sceneState;
        RenderGraphRegistry m_resourceRegistry;
        RenderGraphAllocator m_allocator;

        bool m_supportEMA = false;
        ComPtr<ID3D12Fence> m_sharedFence;
        UINT64 m_fenceValue = 1;

    public:
        explicit RenderGraph(const std::wstring& name = L"Untitled RenderGraph");
        ~RenderGraph() = default;

        ResourceEntry& GetRegisteredResourceEntry(const std::wstring& name);
        bool IsResourceRegistered(const std::wstring& name) const;
        const std::wstring& GetName() const;

        void SetEMASupport(bool emaSupport);

        void Setup();
        void Build();
        void Compile();
        void Execute(GraphicsContext& ctx);
        void Clear();

    public:
        template<typename Ty>
        ResourceEntry RegisterExternalResource(const std::wstring& name, Ty* resource) {
            return m_resourceRegistry.RegisterResource<Ty>(name, resource);
        }

        template<typename Ty>
        Ty* GetRegisteredResource(const std::wstring& name) {
            return m_resourceRegistry.GetRegisteredResource<Ty>(name);
        }


    };
}