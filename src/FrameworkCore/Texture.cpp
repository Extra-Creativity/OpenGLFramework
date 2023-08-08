#include "Texture.h"
#include "Framebuffer.h"
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
extern const char* GetConvertedPath(std::string& buffer, 
    const std::filesystem::path& path);

CPUTextureData::CPUTextureData(const std::filesystem::path& path)
{
    std::string buffer;
    const char* validPath = GetConvertedPath(buffer, path);
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

CPUTextureData GetCPUDataFromAnyTexture(int width, int height, int cpuChannel, 
    int gpuBindTextureType, unsigned int bindTextureID, int gpuSubTextureType)
{
    // This is because stbi uses malloc!
    unsigned char* buffer = reinterpret_cast<unsigned char*>(
        std::malloc(static_cast<size_t>(width) * height * cpuChannel));

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(cpuChannel);

    unsigned int tempFrameBuffer = 0;
    glGenFramebuffers(1, &tempFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, tempFrameBuffer);
    glBindTexture(gpuBindTextureType, bindTextureID);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gpuSubTextureType,
        bindTextureID, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    int initialAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &initialAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, width, height, gpuChannel, GL_UNSIGNED_BYTE, buffer);
    glPixelStorei(GL_PACK_ALIGNMENT, initialAlignment);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindTexture(gpuBindTextureType, 0);
    glDeleteFramebuffers(1, &tempFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return { buffer, width, height, cpuChannel };
}

Texture::Texture(const std::filesystem::path& path) : ID{ 0 }, cpuChannel_{ 0 }
{
    CPUTextureData cpuTextureData{ path };
    if (cpuTextureData.texturePtr == nullptr) [[unlikely]]
        return;
    cpuChannel_ = cpuTextureData.channels;
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

std::pair<int, int> Texture::GetWidthAndHeight()
{
    int width = 0, height = 0;
    glBindTexture(GL_TEXTURE_2D, ID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    return { width, height };
};

CPUTextureData Texture::GetCPUData()
{
    auto [width, height] = GetWidthAndHeight();
    return GetCPUDataFromAnyTexture(width, height, cpuChannel_, 
        GL_TEXTURE_2D, ID, GL_TEXTURE_2D);
}

} // namespace OpenGLFramework::Core
