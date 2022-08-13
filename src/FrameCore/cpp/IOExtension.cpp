#include "IOExtension.h"
#include <sstream>
#include <fstream>
#include <version>
#ifdef __cpp_lib_format	
#include <format>
#endif

namespace IOExtension
{
    std::string ReadAll(std::filesystem::path filePath)
    {
        std::ifstream fin(filePath);
        if (!fin.is_open()) [[unlikely]]
        {
            LogError(std::source_location::current(), "Cannot open " + filePath.string());
            return std::string{};
        }
        std::stringstream buffer;
        buffer << fin.rdbuf();
        return buffer.str(); // because of RVO, we do not use std::move.
    };

    void LogError(const std::source_location& location, const std::string_view& errorInfo, std::ostream& logFile)
    {
        #ifdef __cpp_lib_format	
            logFile << std::format("In file {} - function {} - line {}, Error :\n{}\n",
                location.file_name(), location.function_name(), location.line(), errorInfo);
        #else
            using namespace std::string_literals;
            std::string output = "In file "s + location.file_name() + " - function "s + 
                location.function_name() + " - line "s + std::to_string(location.line()) + 
                " , Error : \n"s + std::string{errorInfo} + "\n"s;
            std::cout << output;
        #endif
        return;
    }
}
