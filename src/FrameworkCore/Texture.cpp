#include "Texture.h"
#include "Framebuffer.h"
#include "Shader.h"
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
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFrameBuffer);
    glBindTexture(gpuBindTextureType, bindTextureID);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        gpuSubTextureType, bindTextureID, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    int initialAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &initialAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, width, height, gpuChannel, GL_UNSIGNED_BYTE, buffer);
    glPixelStorei(GL_PACK_ALIGNMENT, initialAlignment);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindTexture(gpuBindTextureType, 0);
    glDeleteFramebuffers(1, &tempFrameBuffer);

    return { buffer, width, height, cpuChannel };
}

Texture::Texture(const std::filesystem::path& path,
    const TextureParamConfig& paramConfig) : ID_{ 0 }, cpuChannel_{ 0 }
{
    CPUTextureData cpuTextureData{ path };
    if (cpuTextureData.texturePtr == nullptr) [[unlikely]]
        return;
    cpuChannel_ = cpuTextureData.channels;

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(cpuTextureData.channels);
    TextureGenConfig genConfig = GetDefaultTextureGenConfig(gpuChannel);

    glGenTextures(1, &ID_);
    glBindTexture(GL_TEXTURE_2D, ID_);

    genConfig.Apply(TextureType::Texture2D, cpuTextureData);
    paramConfig.Apply();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return;
};

std::pair<int, int> Texture::GetWidthAndHeight() const
{
    int width = 0, height = 0;
    glBindTexture(GL_TEXTURE_2D, ID_);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
    return { width, height };
};

CPUTextureData Texture::GetCPUData() const
{
    auto [width, height] = GetWidthAndHeight();
    return GetCPUDataFromAnyTexture(width, height, cpuChannel_, 
        GL_TEXTURE_2D, ID_, GL_TEXTURE_2D);
}

void Texture::BindTextureOnShader(unsigned int activateID, const char* name,
    const Core::Shader& shader, unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0 + activateID);
    shader.SetInt(name, activateID);
    glBindTexture(GL_TEXTURE_2D, textureID);
};

} // namespace OpenGLFramework::Core
