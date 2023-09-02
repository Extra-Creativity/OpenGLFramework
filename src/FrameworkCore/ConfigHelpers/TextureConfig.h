#pragma once
#include "../../Utility/GLHelper/TypeMapping.h"
#include <glad/glad.h>
#include <utility>
#include <functional>

namespace OpenGLFramework::Core
{

struct CPUTextureData;

enum class TextureType {
    Texture2D = GL_TEXTURE_2D,
    CubeMap = GL_TEXTURE_CUBE_MAP,
    CubeMapPositiveX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    CubeMapNegativeX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    CubeMapPositiveY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    CubeMapNegativeY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    CubeMapPositiveZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    CubeMapNegativeZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

struct TextureGenConfig
{
    GLHelper::GPUInternalFormat gpuPixelFormat = 
        GLHelper::ColorInternalFormat<GLHelper::ColorComponents::RGB>::safe_value;

    enum class CPUPixelFormat {
        R = GL_RED, RG = GL_RG, RGB = GL_RGB,
        BGR = GL_BGR, RGBA = GL_RGBA, BGRA = GL_BGRA,
        Depth = GL_DEPTH_COMPONENT, DepthStencil = GL_DEPTH_STENCIL,
        Stencil = GL_STENCIL_INDEX
    } cpuPixelFormat = CPUPixelFormat::RGB;

    // specify the element type of pixel.
    GLHelper::RawDataType rawDataType = GLHelper::ToGLType<std::uint8_t>::safe_value;

    void Apply(TextureType type, unsigned int width,
        unsigned int height, void* data) const;
    void Apply(TextureType type, const CPUTextureData& data) const;
};

inline TextureGenConfig GetDefaultTextureGenConfig(GLenum gpuChannel)
{
    // cpuPixelFormat is assigned to gpuChannel since it's passed into OpenGL,
    // and their macros are just same.
    return {
        .gpuPixelFormat = static_cast<GLHelper::GPUInternalFormat>(gpuChannel),
        .cpuPixelFormat = static_cast<TextureGenConfig::CPUPixelFormat>(gpuChannel)
    };
}

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
struct TextureParamConfig
{
    TextureType textureType = TextureType::Texture2D;

    enum class MinFilterType {
        Nearest = GL_NEAREST, Linear = GL_LINEAR,
        NearestAfterMIPMAPNearst = GL_NEAREST_MIPMAP_NEAREST,
        LinearAfterMIPMAPNearst = GL_LINEAR_MIPMAP_NEAREST,
        NearestAfterMIPMAPLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearAfterMIPMAPLinear = GL_LINEAR_MIPMAP_LINEAR
    } minFilter = MinFilterType::NearestAfterMIPMAPLinear;

    enum class MaxFilterType {
        Nearest = GL_NEAREST, Linear = GL_LINEAR
    } maxFilter = MaxFilterType::Linear;

    enum class WrapType {
        ClampToEdge = GL_CLAMP_TO_EDGE, ClampToBorder = GL_CLAMP_TO_BORDER,
        MirroredRepeat = GL_MIRRORED_REPEAT, Repeat = GL_REPEAT,
        MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
    } wrapS = WrapType::Repeat, wrapT = WrapType::Repeat,
      wrapR = WrapType::Repeat;

    bool needMIPMAP = false;
    void(*auxHandle)() = nullptr;
    void Apply() const;
};
using TextureConfig = std::pair<TextureGenConfig, TextureParamConfig>;
using TextureConfigCRef = std::pair<std::reference_wrapper<const TextureGenConfig>, 
    std::reference_wrapper<const TextureParamConfig>>;

}
