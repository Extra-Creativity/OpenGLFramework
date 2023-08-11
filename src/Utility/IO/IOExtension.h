#pragma once
#ifndef OPENGLFRAMEWORK_IOEXTENSION_H_
#define OPENGLFRAMEWORK_IOEXTENSION_H_

#include <string>
#include <filesystem>
#include <string_view>
#include <source_location>
#include <iostream>

namespace OpenGLFramework::IOExtension
{

std::string ReadAll(const std::filesystem::path& filePath);
void LogError(std::string_view errorInfo, std::ostream& logFile = std::cerr,
    const std::source_location& location = std::source_location::current());

template<typename CharType = char, typename CharTraits = std::char_traits<CharType>>
void LogStreamStatus(const std::basic_ios<CharType, CharTraits>& stream,
    const std::source_location& location = std::source_location::current())
{
    if (stream.bad())
    {
        const char* errInfo = "I/O error causing bad stream.";
        LogError(errInfo, std::cerr, location);
        throw std::runtime_error{ errInfo };
    }
    else if (stream.fail())
    {
        const char* errInfo = "Wrong format causing failed stream.";
        LogError(errInfo, std::cerr, location);
        throw std::runtime_error{ errInfo };
    }
    return;
};

} // namespace OpenGLFramework::IOExtension
#endif // !OPENGLFRAMEWORK_IOEXTENSION_H_
