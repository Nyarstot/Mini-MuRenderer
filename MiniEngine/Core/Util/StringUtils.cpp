#include "pch.h"
#include "Util/StringUtils.h"
#include <codecvt>
#include <locale>
#include <string>


namespace StringUtils
{
    std::string WideToNarrow(const std::wstring& aWide)
    {
        if (aWide.empty()) return "";
        std::wstring_convert<
            std::codecvt_utf8_utf16<std::wstring::value_type>,
            std::wstring::value_type
        > utf16conv;
        return utf16conv.to_bytes(aWide);
    }
}