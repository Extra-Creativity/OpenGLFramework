#include "Framebuffer.h"
#include "Utility/IO/IOExtension.h"

#include <glad/glad.h>

namespace OpenGLFramework::Core
{

void Framebuffer::GenerateAndAttachTextureBuffer_()
{
    glGenTextures(1, &textureColorBuffer_);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
        textureColorBuffer_, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
   
void Framebuffer::GenerateAndAttachDepthBuffer_()
{
    glGenRenderbuffers(1, &renderBuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
        GL_RENDERBUFFER, renderBuffer_);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Framebuffer::Framebuffer(size_t init_width, size_t init_height, 
    bool needDepthTesting) : width_(init_width), height_(init_height)
{
    glGenFramebuffers(1, &frameBuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

    GenerateAndAttachTextureBuffer_();

    // prepare for depth test.
    if (needDepthTesting)
        GenerateAndAttachDepthBuffer_();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
        [[unlikely]]
        IOExtension::LogError("Framebuffer is not complete.");

    // restore default option.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

Framebuffer::Framebuffer(Framebuffer&& another) noexcept: 
    frameBuffer_(another.frameBuffer_), renderBuffer_(another.renderBuffer_),
    textureColorBuffer_(another.textureColorBuffer_), width_(another.width_), 
    height_(another.height_) 
{
    another.frameBuffer_ = another.textureColorBuffer_ = another.renderBuffer_ = 0;
    return;
}

void Framebuffer::ReleaseResources_()
{
    glDeleteRenderbuffers(1, &renderBuffer_);
    glDeleteTextures(1, &textureColorBuffer_);
    glDeleteBuffers(1, &frameBuffer_);
    return;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& another) noexcept
{
    if (&another == this) [[unlikely]]
        return *this;

    ReleaseResources_();
    frameBuffer_ = another.frameBuffer_;
    renderBuffer_ = another.renderBuffer_;
    textureColorBuffer_ = another.textureColorBuffer_;
    width_ = another.width_;
    height_ = another.height_;
    another.frameBuffer_ = another.textureColorBuffer_ = another.renderBuffer_ = 0;
    return *this;
};

void Framebuffer::Resize(size_t width, size_t height)
{
    width_ = width, height_ = height;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

    glBindTexture(GL_TEXTURE_2D, textureColorBuffer_);
    // note that here we assume texture format is RGB-based.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (renderBuffer_ != 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
            width_, height_);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

Framebuffer::~Framebuffer()
{
    ReleaseResources_();
    return;
}

} // namespace OpenGLFramework::Core