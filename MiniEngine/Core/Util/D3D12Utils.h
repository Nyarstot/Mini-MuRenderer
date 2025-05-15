#pragma once


namespace D3D12Utils
{
    template<typename Ty, typename Tu>
    constexpr Ty AlignUp(Ty size, Tu alignment) {
        return (Ty)(((std::size_t)size + (std::size_t)alignment - 1) & ~((std::size_t)alignment - 1));
    }

    static inline UINT Align(UINT size, UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
}