#include "IOExtension.h"
#include <sstream>
#include <fstream>
#include <version>
#ifdef __cpp_lib_format	
#include <format>
#endif

namespace OpenGLFramework::IOExtension
{

std::string ReadAll(const std::filesystem::path& filePath)
{
    std::ifstream fin(filePath);
    if (!fin.is_open()) [[unlikely]]
    {
        LogError("Cannot open " + filePath.string());
        return std::string{};
    }
    std::stringstream buffer;
    buffer << fin.rdbuf();
    return buffer.str();
};

void LogError(const std::string_view& errorInfo, std::ostream& logFile,
    const std::source_location& location)
{
#ifdef __cpp_lib_format	
    logFile << std::format("In file {} - function {} - line {}, Error :\n{}\n",
        location.file_name(), location.function_name(), location.line(), errorInfo);
#else
    using namespace std::string_literals;
    std::string output = "In file "s + location.file_name() + " - function "s +
        location.function_name() + " - line "s + std::to_string(location.line()) +
        " , Error : \n"s + std::string{ errorInfo } + "\n"s;
    logFile << output;
#endif
    return;
}

} // namespace OpenGLFramework::IOExtension