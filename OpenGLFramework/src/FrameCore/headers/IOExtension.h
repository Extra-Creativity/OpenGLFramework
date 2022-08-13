#pragma once
#ifndef _OPENGLFRAMEWORK_IOEXTENSION_H_
#define _OPENGLFRAMEWORK_IOEXTENSION_H_

#include <string>
#include <filesystem>
#include <string_view>
#include <source_location>
#include <iostream>

namespace IOExtension
{
    std::string ReadAll(std::filesystem::path filePath);
    void LogError(const std::source_location& location, const std::string_view& errorInfo, std::ostream& logFile = std::cerr);
}

#endif // !_OPENGLFRAMEWORK_IOEXTENSION_H_
