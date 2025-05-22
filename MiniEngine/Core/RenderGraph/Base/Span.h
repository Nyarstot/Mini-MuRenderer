#pragma once

#include <initializer_list>
#include <type_traits>
#include <xmemory>


namespace RenderGraph
{
    template <typename T>
    class Span
    {
        using Ty = std::remove_cv_t<T>;
        using TyPtr = Ty*;
        using TyRef = Ty&;

    private:
        TyPtr m_data = nullptr;
        std::size_t m_count;

    public:
        Span(TyPtr data, std::size_t count)
            : m_data(data), m_count(count) {};

        Span(std::initializer_list<Ty> list) {
            // I didn't find any proprate way to use initializer_list in this context.
            m_count = list.size();
            m_data = new Ty[m_count];
            std::move(list.begin(), list.end(), m_data);
        };

        template <typename Container, typename = std::enable_if<
            std::is_convertible<decltype(std::declval<Container>().data()), TyPtr>::value&&
            std::is_convertible<decltype(std::declval<Container>().size()), std::size_t>::value>>
            Span(const Container& container)
            : m_data(container.data()), m_count(container.size()) {};

        template<std::size_t C>
        Span(Ty(&arr)[C]) : m_data(arr), m_count(C) {};

        ~Span() {
            m_data = nullptr;
            m_count = 0;
        }

        Span(const Span&) = default;
        Span& operator=(const Span&) = default;

        TyPtr begin() const {
            ASSERT(m_data, "Span is in an invalid state (dangling?)");
            return m_data;
        }

        TyPtr end() const {
            ASSERT(m_data, "Span is in an invalid state (dangling?)");
            return m_data + m_count;
        }

        std::size_t count() const {
            ASSERT(m_data, "Span is in an invalid state (dangling?)");
            return m_count
        }
    };
}