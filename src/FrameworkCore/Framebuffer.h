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
        .bufferType = RenderBufferConfig::RenderBufferType::Depth,
        .attachmentType = RenderBufferConfig::AttachmentType::Depth
    };

    inline const static TextureParamConfig c_depthTextureDefaultConfig_ = {
        .minFilter = TextureParamConfig::MinFilterType::Nearest,
        .maxFilter = TextureParamConfig::MaxFilterType::Nearest
    };

    inline const static TextureParamConfig c_colorTextureDefaultConfig_ = {
        .minFilter = TextureParamConfig::MinFilterType::Linear,
        .wrapS = TextureParamConfig::WrapType::ClampToEdge,
        .wrapT = TextureParamConfig::WrapType::ClampToEdge,
        .wrapR = TextureParamConfig::WrapType::ClampToEdge
    };

    using TexParamConfigCRef = std::reference_wrapper<const TextureParamConfig>;
    using RenderBufferConfigCRef = std::reference_wrapper<const RenderBufferConfig>;

    struct RenderBuffer { unsigned int buffer; };
    struct RenderTexture { unsigned int buffer; };
    using AttachType = std::variant<RenderBuffer, RenderTexture>;
    unsigned int GetBufferFromAttachType_(const AttachType& buf) const {
        return std::visit([](const auto& arg) { return arg.buffer; }, buf);
    }

public:
    static const auto& GetDepthTextureDefaultParamConfig() { 
        return c_depthTextureDefaultConfig_; 
    }
    static const auto& GetDepthRenderBufferDefaultConfig() { 
        return c_depthBufferDefaultConfig_; 
    }
    static const auto& GetColorTextureDefaultParamConfig() { 
        return c_colorTextureDefaultConfig_;
    }

    glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    enum class BasicClearMode : std::uint32_t { 
        None = 0, ColorClear = 1, DepthClear = 2
    };

    Framebuffer(unsigned int init_width = s_randomLen_,
        unsigned int init_height = s_randomLen_,
        std::variant<std::monostate, RenderBufferConfigCRef, TexParamConfigCRef>
            depthConfig = c_depthBufferDefaultConfig_,
        const std::vector<std::variant<RenderBufferConfigCRef, TexParamConfigCRef>>&
            vec = { c_colorTextureDefaultConfig_ });

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
    
    bool HasColorBuffer() const { return !colorBuffers_.empty(); }
    void SetClearMode(std::initializer_list<BasicClearMode> modes) { 
        clearMode_ = 0;
        for (auto mode : modes)
            clearMode_ |= static_cast<decltype(clearMode_)>(mode);
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
    // Note: this will be reset to all enabled in the ctor.
    std::underlying_type_t<BasicClearMode> clearMode_ = 0;

    void GenerateAndAttachDepthBuffer_(RenderBufferConfigCRef ref);
    void GenerateAndAttachDepthBuffer_(TexParamConfigCRef ref);

    void GenerateAndAttachColorBuffer_(RenderBufferConfigCRef ref, int);
    void GenerateAndAttachColorBuffer_(TexParamConfigCRef ref, int);
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
