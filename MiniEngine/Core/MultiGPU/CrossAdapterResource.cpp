#include "pch.h"
#include "MultiGPU/CrossAdapterResource.h"


namespace MultiGPU
{
    bool CrossAdapterResource::Initialize(MultiAdapterManager* adapterManager, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initialState)
    {
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_RESOURCE_STATES actualInitialState = initialState;

        ASSERT_SUCCEEDED(adapterManager->GetDevice(0)->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_SHARED,
            &desc, actualInitialState, nullptr,
            IID_PPV_ARGS(&m_primaryResource)
        ));

        HANDLE sharedHandle = nullptr;
        ASSERT_SUCCEEDED(adapterManager->GetDevice()->CreateSharedHandle(
            m_primaryResource.Get(), nullptr, GENERIC_ALL, nullptr, &sharedHandle
        ));

        ASSERT_SUCCEEDED(adapterManager->GetDevice(1)->OpenSharedHandle(
            sharedHandle, IID_PPV_ARGS(&m_secondaryResource)
        ));

        return true;
    }

    ID3D12Resource* CrossAdapterResource::GetPrimaryResource() const
    {
        return m_primaryResource.Get();
    }

    ID3D12Resource* CrossAdapterResource::GetSecondaryResource() const
    {
        return m_secondaryResource.Get();
    }
}