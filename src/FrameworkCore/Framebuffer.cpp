#include "Framebuffer.h"
#include "Utility/IO/IOExtension.h"
#include "Texture.h"

#include <glad/glad.h>

namespace OpenGLFramework::Core
{

void Framebuffer::GenerateAndAttachDepthBuffer_(RenderBufferConfigCRef ref)
{
    depthBuffer_ = RenderBuffer{ ref.get().Apply(width_, height_) };
    return;
}

void Framebuffer::GenerateAndAttachDepthBuffer_(TexParamConfigCRef ref)
{
    unsigned int buffer = 0;
    glGenTextures(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, buffer);
    TextureGenConfig{
        .gpuPixelFormat = TextureGenConfig::GPUPixelFormat::Depth,
        .cpuPixelFormat = TextureGenConfig::CPUPixelFormat::Depth,
        .rawDataType = TextureGenConfig::RawDataType::Float
    }.Apply(TextureType::Texture2D, width_, height_, nullptr);
    ref.get().Apply();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, buffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    depthBuffer_ = RenderTexture{ buffer };
    return;
}

void Framebuffer::GenerateAndAttachColorBuffer_(RenderBufferConfigCRef ref, int id) {
    auto realConfig = ref.get();
    realConfig.attachmentType = static_cast<RenderBufferConfig::AttachmentType>(
        GL_COLOR_ATTACHMENT0 + id
    );
    auto buffer = realConfig.Apply(width_, height_);
    colorBuffers_.push_back(RenderBuffer{ buffer });
    return;
};

void Framebuffer::GenerateAndAttachColorBuffer_(TexParamConfigCRef ref, int id) {
    unsigned int buffer = 0;
    glGenTextures(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, buffer);

    GetDefaultTextureGenConfig(GL_RGB).Apply(TextureType::Texture2D,
        width_, height_, nullptr);
    ref.get().Apply();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id,
        GL_TEXTURE_2D, buffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    colorBuffers_.push_back(RenderTexture{ buffer });
};

Framebuffer::Framebuffer(unsigned int init_width, unsigned int init_height,
    std::variant<std::monostate, RenderBufferConfigCRef, TexParamConfigCRef>
        depthConfig,
    const std::vector<std::variant<RenderBufferConfigCRef, TexParamConfigCRef>>&
        colorConfigs): width_{ init_width }, height_{ init_height }
{
    if (colorConfigs.size() > GL_MAX_COLOR_ATTACHMENTS ||
        colorConfigs.size() > GL_MAX_DRAW_BUFFERS) [[unlikely]]
    {
        IOExtension::LogError("Too many color buffers.");
        return;
    }

    glGenFramebuffers(1, &frameBuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);

    if (colorConfigs.empty()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    std::visit([this](auto&& arg) {
        using T = std::remove_cvref_t<decltype(arg)>;
        if constexpr (!std::is_same_v<T, std::monostate>)
            GenerateAndAttachDepthBuffer_(arg);
    }, depthConfig);

    for (int i = 0, len = static_cast<int>(colorConfigs.size()); i < len; i++)
    {
        std::visit([this, i](auto&& arg) {
            GenerateAndAttachColorBuffer_(arg, i);
        }, colorConfigs[i]);
    }

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
    colorBuffers_(std::move(another.colorBuffers_)), width_(another.width_),
    height_(another.height_), clearMode_(another.clearMode_)
{
    another.frameBuffer_ = 0;
    another.depthBuffer_ = RenderBuffer{ 0 };
    return;
}

void Framebuffer::ReleaseResources_()
{
    auto ReleaseBuffer = [](auto&& arg) {
        using T = std::remove_cvref_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, RenderBuffer>)
            glDeleteRenderbuffers(1, &arg.buffer);
        else if constexpr (std::is_same_v<T, RenderTexture>)
            glDeleteTextures(1, &arg.buffer);
        else
        {
            IOExtension::LogError("Unrecognized format.");
            std::terminate();
        }
    };
    std::visit(ReleaseBuffer, depthBuffer_);
    for (auto& buffer : colorBuffers_)
        std::visit(ReleaseBuffer, buffer);
    glDeleteFramebuffers(1, &frameBuffer_);
    colorBuffers_.clear();
    return;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& another) noexcept
{
    if (&another == this) [[unlikely]]
        return *this;

    ReleaseResources_();
    frameBuffer_ = another.frameBuffer_;
    depthBuffer_ = std::move(another.depthBuffer_);
    colorBuffers_ = std::move(another.colorBuffers_);
    width_ = another.width_;
    height_ = another.height_;
    another.frameBuffer_ = 0;
    another.depthBuffer_ = RenderBuffer{ 0 };
    return *this;
};

Framebuffer::~Framebuffer()
{
    ReleaseResources_();
    return;
}

void Framebuffer::UseAsRenderTarget() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_);
    if (GetDepthBuffer() != 0)
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
    std::vector<unsigned char> pixelBuffer(
        static_cast<size_t>(width) * height * channelNum);
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