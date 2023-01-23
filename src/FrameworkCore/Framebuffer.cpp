#include "Framebuffer.h"
#include "Utility/IO/IOExtension.h"
#include <glad/glad.h>

namespace OpenGLFramework::Core
{

Framebuffer::Framebuffer(size_t init_width, size_t init_height, bool needDepthTesting) :
    m_width(init_width), m_height(init_height)
{
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    // prepare for depth test.
    if (needDepthTesting)
    {
        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) [[unlikely]]
        IOExtension::LogError("Framebuffer is not complete.");

    // restore default option.
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

void Framebuffer::resize(size_t width, size_t height)
{
    m_width = width, m_height = height;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    // note that here we assume texture format is RGB-based.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (renderBuffer != 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    return;
}

Framebuffer::~Framebuffer()
{
    glDeleteRenderbuffers(1, &renderBuffer);
    glDeleteTextures(1, &textureColorBuffer);
    glDeleteBuffers(1, &frameBuffer);
    return;
}

} // namespace OpenGLFramework::Core