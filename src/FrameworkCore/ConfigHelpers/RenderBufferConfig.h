#pragma once
#include "../../Utility/GLHelper/TypeMapping.h"
#include <glad/glad.h>
#include <cstdint>
#include <limits>

namespace OpenGLFramework::Core
{

struct RenderBufferConfig
{
    // NOTICE: Some color internal formats are not available in render buffer. See
    // https://registry.khronos.org/OpenGL/specs/gl/glspec46.core.pdf P208-P210
    // for all allowed formats. Briefly, color formats that use 16/32/64/128 bits
    // and aren't SNorm are supported. For example, R, RG, RGBA SInt/UInt/Float.
    // For depth and stencil, size should be designated explicitly.
    GLHelper::GPUInternalFormat bufferType;

    enum class AttachmentType : std::uint32_t {
        Depth = GL_DEPTH_ATTACHMENT,
        DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
        Color = std::numeric_limits<std::uint32_t>::max()
    } attachmentType;

    unsigned int Apply(unsigned int width, unsigned int height) const;
};

}

