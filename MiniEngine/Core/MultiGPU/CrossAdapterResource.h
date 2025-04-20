#pragma once

#include "MultiGPU/MultiAdapterManager.h"


using namespace Microsoft::WRL;

namespace MultiGPU
{
    class CrossAdapterResource
    {
    private:
        ComPtr<ID3D12Resource> m_primaryResource;
        ComPtr<ID3D12Resource> m_secondaryResource;

    public:
        CrossAdapterResource() = default;
        ~CrossAdapterResource() = default;

        bool Initialize(MultiAdapterManager* adapterManager, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initialState);
        ID3D12Resource* GetPrimaryResource() const;
        ID3D12Resource* GetSecondaryResource() const;

    };
}