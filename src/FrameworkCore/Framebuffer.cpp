#include "Framebuffer.h"
#include "Utility/IO/IOExtension.h"
#include "Texture.h"

#include <glad/glad.h>

namespace OpenGLFramework::Core
{

void Framebuffer::GenerateAndAttachTextureBuffer_()
{
    glGenTextures(1, &textureColorBuffer_);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer_);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
        textureColorBuffer_, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
   
void Framebuffer::GenerateAndAttachWriteOnlyDepthBuffer_()
{
    glGenRenderbuffers(1, &depthBuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
        GL_RENDERBUFFER, depthBuffer_);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::GenerateAndAttachReadableDepthBuffer_()
{
    glGenTextures(1, &depthBuffer_);
    glBindTexture(GL_TEXTURE_2D, depthBuffer_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer_, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::GenerateAdditionalBuffers_(unsigned int begin)
{
    glGenTextures(static_cast<int>(additionalBuffers_.size()),
        additionalBuffers_.data());
    for (auto& buffer : additionalBuffers_)
    {
        glBindTexture(GL_TEXTURE_2D, buffer);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB,
            GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + begin, GL_TEXTURE_2D,
            textureColorBuffer_, 0);
        begin++;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return;
}

Framebuffer::Framebuffer(unsigned int init_width, unsigned int init_height,
    BasicBufferType type, int additionalBufferAmount) :
    basicBufferType_(type), additionalBuffers_(additionalBufferAmount), 
    width_(init_width), height_(init_height)
{
    glGenFramebuffers(1, &frameBuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

    // In most occasions, we only need one color texture.
    int colorTextureBeginID = 1;
    switch (type)
    {
        using enum BasicBufferType;
    case OnlyColorBuffer:
        GenerateAndAttachTextureBuffer_();
        break;
    case OnlyReadableDepthBuffer:
        GenerateAndAttachReadableDepthBuffer_();
        if (additionalBufferAmount == 0)
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
        // no color texture now.
        colorTextureBeginID = 0;
        break;
    case ColorBufferAndWriteOnlyDepthBuffer:
        GenerateAndAttachTextureBuffer_();
        GenerateAndAttachWriteOnlyDepthBuffer_();
        break;
    case ColorBufferAndReadableDepthBuffer:
        GenerateAndAttachTextureBuffer_();
        GenerateAndAttachReadableDepthBuffer_();
        break;
    default:
        IOExtension::LogError("Unrecognized basic buffer format.");
        return;
    }

    GenerateAdditionalBuffers_(colorTextureBeginID);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
        [[unlikely]]
        IOExtension::LogError("Framebuffer is not complete.");

    using enum BasicClearMode;
    SetClearMode({ DepthClear, ColorClear });
    // restore default option.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

Framebuffer::Framebuffer(Framebuffer&& another) noexcept: 
    frameBuffer_(another.frameBuffer_), depthBuffer_(another.depthBuffer_),
    textureColorBuffer_(another.textureColorBuffer_), 
    basicBufferType_(another.basicBufferType_), 
    additionalBuffers_(std::move(another.additionalBuffers_)),
    width_(another.width_), height_(another.height_), clearMode_(another.clearMode_)
{
    another.frameBuffer_ = another.textureColorBuffer_ = another.depthBuffer_ = 0;
    return;
}

void Framebuffer::ReleaseResources_()
{
    using enum BasicBufferType;
    if (basicBufferType_ == ColorBufferAndWriteOnlyDepthBuffer)
        glDeleteRenderbuffers(1, &depthBuffer_);
    else
        glDeleteTextures(1, &depthBuffer_);
    glDeleteTextures(1, &textureColorBuffer_);
    glDeleteTextures(static_cast<int>(additionalBuffers_.size()), additionalBuffers_.data());
    glDeleteBuffers(1, &frameBuffer_);
    return;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& another) noexcept
{
    if (&another == this) [[unlikely]]
        return *this;

    ReleaseResources_();
    frameBuffer_ = another.frameBuffer_;
    depthBuffer_ = another.depthBuffer_;
    textureColorBuffer_ = another.textureColorBuffer_;
    basicBufferType_ = another.basicBufferType_;
    width_ = another.width_;
    height_ = another.height_;
    additionalBuffers_ = std::move(another.additionalBuffers_);
    another.frameBuffer_ = another.textureColorBuffer_ = another.depthBuffer_ = 0;
    return *this;
};

void Framebuffer::Resize(unsigned int width, unsigned int height)
{
    width_ = width, height_ = height;
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

    glBindTexture(GL_TEXTURE_2D, textureColorBuffer_);
    // note that here we assume texture format is RGB-based.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, 
        GL_UNSIGNED_BYTE, nullptr);

    for (auto& buffer : additionalBuffers_)
    {
        glBindTexture(GL_TEXTURE_2D, buffer);
        // note that here we assume texture format is RGB-based.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width_, height_, 0, GL_RGB,
            GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    if (depthBuffer_ != 0)
    {
        if (basicBufferType_ == BasicBufferType::ColorBufferAndWriteOnlyDepthBuffer)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer_);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                width_, height_);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, depthBuffer_);
            // note that here we assume texture format is RGB-based.
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                width_, height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

Framebuffer::~Framebuffer()
{
    ReleaseResources_();
    return;
}

void Framebuffer::UseAsRenderTarget()
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);
    if (NeedDepthTesting())
    {
        if (clearMode_ & static_cast<decltype(clearMode_)>(BasicClearMode::DepthClear))
            glClear(GL_DEPTH_BUFFER_BIT);
    }
    else
        glDisable(GL_DEPTH_TEST);
    if (clearMode_ & static_cast<decltype(clearMode_)>(BasicClearMode::ColorClear))
    {
        auto& color = backgroundColor;
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    return;
};

void Framebuffer::RestoreDefaultRenderTarget()
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
};

std::vector<unsigned char> Framebuffer::SaveFrameBufferInCPU(unsigned int bufferID,
    unsigned int width, unsigned int height, int channelNum)
{
    std::vector<unsigned char> pixelBuffer(width * height * channelNum);
    GLenum gpuChannel = GetGPUChannelFromCPUChannel(channelNum);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, bufferID);

    int initialAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &initialAlignment);    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, gpuChannel, GL_UNSIGNED_BYTE,
        pixelBuffer.data());
    glPixelStorei(GL_PACK_ALIGNMENT, initialAlignment);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return pixelBuffer;
}

} // namespace OpenGLFramework::Core