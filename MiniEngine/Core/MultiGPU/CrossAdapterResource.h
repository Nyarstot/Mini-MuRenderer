#pragma once

#include "GpuResource.h"
#include "BufferManager.h"
#include "Util/D3D12Utils.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class CrossAdapterResource final
    {
    private:
        std::shared_ptr<ColorBuffer> m_primaryResource;
        std::shared_ptr<ColorBuffer> m_secondaryResource;
        ComPtr<ID3D12Heap> m_heaps[2];

    public:
        CrossAdapterResource()
            : m_primaryResource(new ColorBuffer()), m_secondaryResource(new ColorBuffer())
        {
        }

        ~CrossAdapterResource() = default;

        void Create(ID3D12Device* PrimaryDevice, ID3D12Device* SecondaryDevice, GpuResource& ParentResource);
        void Reset();

        ColorBuffer* GetPrimaryResource() const { return m_primaryResource.get(); }
        ColorBuffer& GetPrimaryResourceRef() const { return *m_primaryResource.get(); }

        ColorBuffer* GetSecondaryResource() const { return m_secondaryResource.get(); }
        ColorBuffer& GetSecondaryResourceRef() const { return *m_secondaryResource.get(); }

    };
}
