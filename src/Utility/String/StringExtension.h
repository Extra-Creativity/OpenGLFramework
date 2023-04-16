#pragma once
#ifndef OPENGLFRAMEWORK_STRINGEXTENSION_H_
#define OPENGLFRAMEWORK_STRINGEXTENSION_H_

#include <ranges>
#include <algorithm>
#include <locale>
#include <string>
#include <string_view>

namespace OpenGLFramework::StringExtension {

template<typename CharType>
CharType CharAsciiToLower(CharType ch)
{
    return std::tolower(ch, std::locale::classic());
}

template<typename CharType>
std::basic_string<CharType> StringAsciiToLower(
    std::basic_string_view<CharType> input)
{
    // in C++23, we can use .resize_and_overwrite to reduce overhead.
    std::basic_string<CharType> result(input.size(), 0);
    std::transform(input.begin(), input.end(), result.begin(), 
        CharAsciiToLower<CharType>);
    return result;
}

template<typename StringType>
concept EnableSubstr = requires(StringType str, size_t begin, size_t cnt) {
    requires std::ranges::random_access_range<StringType>;
    str.substr(begin, cnt);
};

template<EnableSubstr StringType>
StringType TrimBegin(const StringType& str)
{
    auto chIsBlank = [](decltype(*str.begin()) ch) -> bool { 
        return static_cast<int>(ch) < 128 && 
            !std::isgraph(ch, std::locale::classic());
    };
    auto beginPosIt = std::find_if_not(str.begin(), str.end(), chIsBlank);

    size_t beginPos = beginPosIt == str.end() ? 0 : beginPosIt - str.begin();
    return str.substr(beginPos);
}

template<EnableSubstr StringType>
StringType TrimEnd(const StringType& str)
{
    auto chIsBlank = [](decltype(*str.begin()) ch) -> bool {
        return static_cast<int>(ch) < 128 && 
            !std::isgraph(ch, std::locale::classic());
    };
    auto endPosIt = std::find_if_not(str.rbegin(), str.rend(), chIsBlank);

    size_t endPos = endPosIt == str.rbegin() ? str.size() : str.rend() - endPosIt;
    return str.substr(0, endPos);
}

template<EnableSubstr StringType>
StringType Trim(const StringType& str)
{
    auto chIsBlank = [](decltype(*str.begin()) ch) -> bool {
        return static_cast<int>(ch) < 128 && 
            !std::isgraph(ch, std::locale::classic());
    };
    auto beginPosIt = std::find_if_not(str.begin(), str.end(), chIsBlank);
    auto endPosIt = std::find_if_not(str.rbegin(), str.rend(), chIsBlank);

    size_t beginPos = beginPosIt == str.end() ? 0 : beginPosIt - str.begin(),
        endPos = endPosIt == str.rbegin() ? str.size() : str.rend() - endPosIt;

    return str.substr(beginPos, endPos - beginPos);
}

} // namespace OpenGLFramework::StringExtension

#endif // !OPENGLFRAMEWORK_STRINGEXTENSION_H_