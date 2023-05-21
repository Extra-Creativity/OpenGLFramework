#ifdef _MSC_VER
// disable external library warnings.
#pragma warning(push, 0)
#pragma warning(disable: 26819 6385 26451 26812 6386 6262)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <string>
#include <filesystem>

#ifdef _MSC_VER
// restore warnings.
#pragma warning(pop)
#endif

static const int g_fileMaxLen = 1024;

namespace OpenGLFramework::Core
{

const char* GetConvertedPath(std::string& buffer, const std::filesystem::path& path)
{
#ifdef _WIN32
    buffer.resize(g_fileMaxLen, '\0');
    stbi_convert_wchar_to_utf8(buffer.data(), g_fileMaxLen, path.c_str());
    return buffer.c_str();
#else
    return path.c_str();
#endif // _WIN32
}

}