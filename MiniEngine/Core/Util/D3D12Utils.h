#pragma once


namespace D3D12Utils
{
    template<typename Ty, typename U>
    constexpr Ty AlignUp(Ty size, U alignment) {
        return (Ty)(((size_t)size + (size_t)alignment - 1) & ~((size_t)alignment - 1));
    }

    static inline UINT Align(UINT Size, UINT Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) {
        return (Size + Alignment - 1) & ~(Alignment - 1);
    }
}