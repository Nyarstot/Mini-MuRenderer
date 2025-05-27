#pragma once


namespace RenderGraph
{
    class RenderGraphAllocator final
    {
    private:
        std::unique_ptr<byte[]> m_baseAddress;
        byte* m_ptr;
        byte* m_sentinel;
        std::size_t m_currentMemoryUsage;

    public:
        explicit RenderGraphAllocator(std::size_t sizeInBytes);
        ~RenderGraphAllocator() = default;

        void* Allocate(std::size_t sizeInBytes, std::size_t alignment);
        void Reset();

    public:
        template<typename Ty, typename... TArgs>
        Ty* Construct(TArgs&&... Args)
        {
            void* memory = Allocate(sizeof(Ty), 16);
            return new (memory)Ty(std::forward<TArgs>(Args)...);
        }

        template<typename Ty>
        static void Destruct(Ty* ptr)
        {
            ptr->~Ty();
        }

    };
}
