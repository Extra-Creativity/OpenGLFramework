#include "Texture.h"
#include "Utility/IO/IOExtension.h"

#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <version>
#ifdef __cpp_lib_format	
#include <format>
#endif

static const int g_fileMaxLen = 1024;

namespace OpenGLFramework::Core
{

CPUTextureData::CPUTextureData(const std::filesystem::path& path)
{
#ifdef _WIN32
    std::string stbiPath(g_fileMaxLen, '\0');
    stbi_convert_wchar_to_utf8(stbiPath.data(), g_fileMaxLen, path.c_str());
    const char* validPath = stbiPath.c_str();
#else
    const char* validPath = path.c_str();
#endif // _WIN32

    texturePtr = stbi_load(validPath, &width, &height, &channels, 0);
    if (texturePtr == nullptr) [[unlikely]]
    {
#   ifdef __cpp_lib_format	
        IOExtension::LogError(std::format("Fail to load texture image at "
            "path {} for reason : {}", path.string(), stbi_failure_reason()));
#   else
        IOExtension::LogError("Fail to load texture image at path "
            + path.string() + " for reason : " + stbi_failure_reason());
#   endif
    }
    return;
}

CPUTextureData::CPUTextureData(CPUTextureData&& another) noexcept:
    texturePtr{ another.texturePtr }, width{ another.width },
    height{ another.height }, channels{ another.channels }
{
    another.texturePtr = nullptr;
    return;
}

CPUTextureData& CPUTextureData::operator=(CPUTextureData&& another) noexcept
{
    if (&another == this) [[unlikely]]
        return *this;

    stbi_image_free(texturePtr);
    texturePtr = another.texturePtr;
    width = another.width, height = another.height, channels = another.channels;

    another.texturePtr = nullptr;
    return *this;
}

CPUTextureData::~CPUTextureData()
{
    stbi_image_free(texturePtr);
}

GLenum GetGPUChannelFromCPUChannel(int cpuChannel)
{
    GLenum gpuChannel;
    switch (cpuChannel)
    {
    case STBI_grey:
        gpuChannel = GL_RED;
        break;
    case STBI_rgb:
        gpuChannel = GL_RGB;
        break;
    case STBI_rgb_alpha:
        gpuChannel = GL_RGBA;
        break;
    default: [[unlikely]]
#   ifdef __cpp_lib_format	
        IOExtension::LogError(
            std::format("Unknown image channel type : {}", cpuChannel));
#   else
        IOExtension::LogError(
            "Unknown image channel type : " + std::to_string(cpuChannel));
#   endif
        gpuChannel = GL_RGBA;
        break;
    }
    return gpuChannel;
}

Texture::Texture(const std::filesystem::path& path) : ID(0)
{
    CPUTextureData cpuTextureData{ path };
    if (cpuTextureData.texturePtr == nullptr) [[unlikely]]
        return;
    GLenum gpuChannel = GetGPUChannelFromCPUChannel(cpuTextureData.channels);

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, gpuChannel, cpuTextureData.width, 
        cpuTextureData.height, 0, gpuChannel, GL_UNSIGNED_BYTE, 
        cpuTextureData.texturePtr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);
    return;
};

} // namespace OpenGLFramework::Core
