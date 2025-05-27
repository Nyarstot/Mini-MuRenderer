#include "pch.h"
#include "RenderGraph/RenderGraphAllocator.h"
#include "Util/D3D12Utils.h"


namespace RenderGraph
{
    RenderGraphAllocator::RenderGraphAllocator(std::size_t sizeInBytes)
        : m_baseAddress(std::make_unique<byte[]>(sizeInBytes)),
        m_ptr(m_baseAddress.get()),
        m_sentinel(m_ptr + sizeInBytes),
        m_currentMemoryUsage(0)
    {
    }

    void* RenderGraphAllocator::Allocate(std::size_t sizeInBytes, std::size_t alignment)
    {
        sizeInBytes = D3D12Utils::AlignUp(sizeInBytes, alignment);
        ASSERT(m_ptr + sizeInBytes <= m_sentinel);
        byte* result = m_ptr + sizeInBytes;

        m_ptr += sizeInBytes;
        m_currentMemoryUsage += sizeInBytes;
        return result;
    }

    void RenderGraphAllocator::Reset()
    {
        m_ptr = m_baseAddress.get();
        m_currentMemoryUsage = 0;
    }
}
