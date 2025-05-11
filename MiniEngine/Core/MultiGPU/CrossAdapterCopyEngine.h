#pragma once

#include "GraphicsCore.h"
#include "MultiGPU/MultiAdapterManager.h"


using namespace Microsoft::WRL;
namespace MultiGPU
{
    class CrossAdapterCopyEngine final
    {
    private:
        ComPtr<ID3D12Heap> m_crossAdapterResourceHeaps[2];
        MultiAdapterManager m_multiAdapterManager;


    private:
        void Initialize();

    public:
        CrossAdapterCopyEngine(const MultiAdapterManager& multiAdapterManager);
        ~CrossAdapterCopyEngine();



    };
}