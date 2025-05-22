#pragma once


namespace D3D12Utils
{
    static inline UINT Align(UINT Size, UINT Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) {
        return (Size + Alignment - 1) & ~(Alignment - 1);
    }
}