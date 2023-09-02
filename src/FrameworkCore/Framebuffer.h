#pragma once

#include "ConfigHelpers/TextureConfig.h"
#include "ConfigHelpers/RenderBufferConfig.h"

#include <glm/glm.hpp>

#include <vector>
#include <variant>
#include <cstdint>
#include <functional> // for reference wrapper.

namespace OpenGLFramework::Core
{
class Framebuffer
{
    inline const static RenderBufferConfig c_depthBufferDefaultConfig_ = {
        .bufferType = GLHelper::SpecialInternalFormat<
            GLHelper::SpecialComponents::D24>::safe_value,
        .attachmentType = RenderBufferConfig::AttachmentType::Depth
    };

    inline const static TextureGenConfig c_depthTexGenDefaultConfig_ = {
        .gpuPixelFormat = GLHelper::SpecialInternalFormat<
            GLHelper::SpecialComponents::Depth>::safe_value,
        .cpuPixelFormat = TextureGenConfig::CPUPixelFormat::Depth
    };

    inline const static TextureParamConfig c_depthTexParamDefaultConfig_ = {
        .minFilter = TextureParamConfig::MinFilterType::Nearest,
        .maxFilter = TextureParamConfig::MaxFilterType::Nearest
    };

    inline const static RenderBufferConfig c_colorBufferDefaultConfig_ = {
        .bufferType = GLHelper::ColorInternalFormat<
            GLHelper::ColorComponents::RGBA>::safe_value,
        .attachmentType = RenderBufferConfig::AttachmentType::Color
    };

    inline const static TextureGenConfig c_colorTexGenDefaultConfig_{};

    inline const static TextureParamConfig c_colorTexParamDefaultConfig_ = {
        .minFilter = TextureParamConfig::MinFilterType::Linear,
        .wrapS = TextureParamConfig::WrapType::ClampToEdge,
        .wrapT = TextureParamConfig::WrapType::ClampToEdge,
        .wrapR = TextureParamConfig::WrapType::ClampToEdge
    };

    using RenderBufferConfigCRef = std::reference_wrapper<const RenderBufferConfig>;
    struct RenderBuffer { unsigned int buffer; };
    struct RenderTexture { unsigned int buffer; };
    using AttachType = std::variant<RenderBuffer, RenderTexture>;
    unsigned int GetBufferFromAttachType_(const AttachType& buf) const {
        return std::visit([](const auto& arg) { return arg.buffer; }, buf);
    }

public:
    using ConfigType = std::variant<RenderBufferConfigCRef, TextureConfigCRef>;
    static TextureConfigCRef GetDepthTextureDefaultConfig() { 
        return { c_depthTexGenDefaultConfig_, c_depthTexParamDefaultConfig_ };
    }
    static const auto& GetDepthRenderBufferDefaultConfig() { 
        return c_depthBufferDefaultConfig_; 
    }
    static TextureConfigCRef GetColorTextureDefaultConfig() {
        return { c_colorTexGenDefaultConfig_, c_colorTexParamDefaultConfig_ };
    }
    static const auto& GetColorRenderBufferDefaultConfig() {
        return c_colorBufferDefaultConfig_;
    }

    glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    enum class BasicClearMode { 
        ColorClear = GL_COLOR_BUFFER_BIT, DepthClear = GL_DEPTH_BUFFER_BIT,
        StencilClear = GL_STENCIL_BUFFER_BIT
    };

    Framebuffer(unsigned int init_width = s_randomLen_,
        unsigned int init_height = s_randomLen_,
        std::variant<std::monostate, RenderBufferConfigCRef, TextureConfigCRef>
            depthConfig = c_depthBufferDefaultConfig_,
        const std::vector<ConfigType>& vec = { GetColorTextureDefaultConfig() });

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& another) noexcept;
    Framebuffer& operator=(Framebuffer&& another) noexcept;
    ~Framebuffer();

    static std::vector<unsigned char> SaveFrameBufferInCPU(unsigned int bufferID,
        unsigned int width, unsigned int height, int channelNum);

    unsigned int GetWidth() const { return width_; }
    unsigned int GetHeight() const { return height_; }
    float GetAspect() const { return static_cast<float>(width_) / height_; }
    unsigned int GetFramebuffer() const { return frameBuffer_; }
    unsigned int GetDepthBuffer() const {
        return GetBufferFromAttachType_(depthBuffer_);
    }
    unsigned int GetColorBuffer(size_t index = 0) const { 
        return GetBufferFromAttachType_(colorBuffers_.at(index));
    }
    auto GetColorBufferNum() const { return colorBuffers_.size(); }
    
    bool HasColorBuffer() const { return !colorBuffers_.empty(); }
    void Clear(std::initializer_list<BasicClearMode> modes = { BasicClearMode::ColorClear,
        BasicClearMode::DepthClear}, bool resetColor = true)
    {
        UseAsRenderTarget();
        std::underlying_type_t<BasicClearMode> clearMode = 0;
        for (auto mode : modes)
            clearMode |= static_cast<decltype(clearMode)>(mode);
        if(resetColor)
            glClearColor(backgroundColor.r, backgroundColor.g,
                backgroundColor.b, backgroundColor.a);
        glClear(clearMode);
        RestoreDefaultRenderTarget();
        return;
    }
    void UseAsRenderTarget() const;
    static void RestoreDefaultRenderTarget();
private:
    unsigned int frameBuffer_ = 0;
    AttachType depthBuffer_;
    std::vector<AttachType> colorBuffers_;

    unsigned int width_;
    unsigned int height_;
    static const unsigned int s_randomLen_ = 1000u;

    void GenerateAndAttachDepthBuffer_(RenderBufferConfigCRef ref);
    void GenerateAndAttachDepthBuffer_(TextureConfigCRef ref);

    void GenerateAndAttachColorBuffer_(RenderBufferConfigCRef ref, int);
    void GenerateAndAttachColorBuffer_(TextureConfigCRef ref, int);
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
