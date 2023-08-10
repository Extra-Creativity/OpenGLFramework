#include "RenderBufferConfig.h"

#include <type_traits>

namespace OpenGLFramework::Core
{

template<typename Enum>
constexpr auto to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
};

unsigned int RenderBufferConfig::Apply(unsigned int width, unsigned int height) const
{
    unsigned int buffer = 0;
    glGenRenderbuffers(1, &buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, to_underlying(bufferType), width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, to_underlying(attachmentType),
        GL_RENDERBUFFER, buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return buffer;
};

}
