#pragma once
#ifndef OPENGLFRAMEWORK_CORE_TEXTURECONFIG_H_
#define OPENGLFRAMEWORK_CORE_TEXTURECONFIG_H_

#include <glad/glad.h>

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
    // We may introduce sized pixel format in the future.
    enum class GPUPixelFormat {
        R = GL_RED, RG = GL_RG, RGB = GL_RGB, RGBA = GL_RGBA,
        Depth = GL_DEPTH_COMPONENT, DepthStencil = GL_DEPTH_STENCIL
    } gpuPixelFormat = GPUPixelFormat::RGB;

    enum class CPUPixelFormat {
        R = GL_RED, RG = GL_RG, RGB = GL_RGB,
        BGR = GL_BGR, RGBA = GL_RGBA, BGRA = GL_BGRA,
    } cpuPixelFormat = CPUPixelFormat::RGB;

    // specify the element type of pixel.
    enum class RawDataType {
        UByte = GL_UNSIGNED_BYTE, Float = GL_FLOAT
    } rawDataType = RawDataType::UByte;

    void Apply(TextureType type, unsigned int width,
        unsigned int height, void* data) const;
    void Apply(TextureType type, const CPUTextureData& data) const;
};

inline TextureGenConfig GetDefaultTextureGenConfig(GLenum gpuChannel)
{
    // cpuPixelFormat is assigned to gpuChannel since it's passed into OpenGL,
    // and their macros are just same.
    return {
        .gpuPixelFormat = static_cast<TextureGenConfig::GPUPixelFormat>(gpuChannel),
        .cpuPixelFormat = static_cast<TextureGenConfig::CPUPixelFormat>(gpuChannel),
        .rawDataType = static_cast<TextureGenConfig::RawDataType>(GL_UNSIGNED_BYTE)
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
    void Apply() const;
};

}

#endif // OPENGLFRAMEWORK_CORE_TEXTURECONFIG_H_