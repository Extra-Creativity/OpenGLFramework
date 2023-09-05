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

void Framebuffer::GenerateAndAttachDepthBuffer_(TextureConfigCRef config)
{
    unsigned int buffer = 0;
    glGenTextures(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, buffer);
    auto& genConfig = config.first.get();
    genConfig.Apply(TextureType::Texture2D, width_, height_, nullptr);
    config.second.get().Apply();

    GLenum attachment;
    switch (genConfig.cpuPixelFormat)
    {
        using enum TextureGenConfig::CPUPixelFormat;
    default:
        IOExtension::LogError("Unknown cpu pixel format for depth buffer,"
            "use depth attachment by default.");
        [[fallthrough]];
    [[likely]] case Depth:
        attachment = GL_DEPTH_ATTACHMENT;
        break;
    [[likely]] case DepthStencil:
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        break;
    case Stencil:
        attachment = GL_STENCIL_ATTACHMENT;
        break;
    };
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, buffer, 0);
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

void Framebuffer::GenerateAndAttachColorBuffer_(TextureConfigCRef config, int id) {
    unsigned int buffer = 0;
    glGenTextures(1, &buffer);
    glBindTexture(GL_TEXTURE_2D, buffer);

    config.first.get().Apply(TextureType::Texture2D, width_, height_, nullptr);
    config.second.get().Apply();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + id,
        GL_TEXTURE_2D, buffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    colorBuffers_.push_back(RenderTexture{ buffer });
};

static std::vector<unsigned int> attachmentIDs;

Framebuffer::Framebuffer(unsigned int init_width, unsigned int init_height,
    std::variant<std::monostate, RenderBufferConfigCRef, TextureConfigCRef>
        depthConfig,
    const std::vector<std::variant<RenderBufferConfigCRef, TextureConfigCRef>>&
        colorConfigs): width_{ init_width }, height_{ init_height }
{
    static unsigned int maxColorAttachments = []() {
        int num = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &num);
        return num < 0 ? 0u : static_cast<unsigned int>(num);
    }();

    static unsigned int maxDrawBuffers = []() {
        int num = 0;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &num);
        if(num < 0)
            return 0u;
        attachmentIDs.reserve(num);
        for (int i = 0; i < num; i++)
            attachmentIDs.push_back(GL_COLOR_ATTACHMENT0 + i);
        return static_cast<unsigned int>(num);
    }();

    if (colorConfigs.size() > maxColorAttachments ||
        colorConfigs.size() > maxDrawBuffers) [[unlikely]]
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

    if (auto attachmentNum = colorConfigs.size(); attachmentNum > 1)
        glDrawBuffers(static_cast<GLsizei>(attachmentNum), attachmentIDs.data());

    // restore default option.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
}

Framebuffer::Framebuffer(Framebuffer&& another) noexcept: 
    frameBuffer_(another.frameBuffer_), depthBuffer_(another.depthBuffer_),
    colorBuffers_(std::move(another.colorBuffers_)), width_(another.width_),
    height_(another.height_)
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