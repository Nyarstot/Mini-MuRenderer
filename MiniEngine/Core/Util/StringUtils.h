#pragma once


namespace StringUtils
{
    std::wstring NarrowToWide(const std::string& aNarrow);
    std::string WideToNarrow(const std::wstring& aWide);
}