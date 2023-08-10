#include "TextureConfig.h"
#include "../Texture.h"

#include <type_traits>

namespace OpenGLFramework::Core
{
template<typename Enum>
constexpr auto to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
};

void TextureGenConfig::Apply(TextureType textureType,
    unsigned int width, unsigned int height, void* data) const
{
    auto type = to_underlying(textureType);

    glTexImage2D(type, 0, to_underlying(gpuPixelFormat), width, height, 0,
        to_underlying(cpuPixelFormat), to_underlying(rawDataType), data);
}

void TextureGenConfig::Apply(TextureType type, const CPUTextureData& data) const
{
    Apply(type, data.width, data.height, data.texturePtr);
};

void TextureParamConfig::Apply() const
{
    auto type = to_underlying(textureType);
    if (needMIPMAP)
        glGenerateMipmap(type);

    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, to_underlying(minFilter));
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, to_underlying(maxFilter));
    glTexParameteri(type, GL_TEXTURE_WRAP_S, to_underlying(wrapS));
    glTexParameteri(type, GL_TEXTURE_WRAP_T, to_underlying(wrapT));
    glTexParameteri(type, GL_TEXTURE_WRAP_R, to_underlying(wrapR));
}

}
