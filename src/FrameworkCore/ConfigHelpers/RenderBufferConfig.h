#pragma once
#ifndef OPENGLFRAMEWORK_CORE_RENDERBUFFERCONFIG_H_
#define OPENGLFRAMEWORK_CORE_RENDERBUFFERCONFIG_H_

#include <glad/glad.h>
#include <cstdint>
#include <limits>

namespace OpenGLFramework::Core
{

struct RenderBufferConfig
{
    // We may introduced more types;
    // See https://www.khronos.org/opengl/wiki/Image_Format for details.
    enum class RenderBufferType {
        Depth = GL_DEPTH_COMPONENT24,
        DepthStencil = GL_DEPTH24_STENCIL8,
        RGBA = GL_RGBA32F
    } bufferType;

    enum class AttachmentType : std::uint32_t {
        Depth = GL_DEPTH_ATTACHMENT,
        DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
        Color = std::numeric_limits<std::uint32_t>::max()
    } attachmentType;

    unsigned int Apply(unsigned int width, unsigned int height) const;
};

}

#endif // OPENGLFRAMEWORK_CORE_RENDERBUFFERCONFIG_H_