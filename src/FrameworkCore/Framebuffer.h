#pragma once
#ifndef OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_
#define OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_

#include <glm/glm.hpp>
#include <vector>

namespace OpenGLFramework::Core
{

class Framebuffer
{
public:
    glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    enum class BasicBufferType { OnlyColorBuffer, OnlyReadableDepthBuffer,
       ColorBufferAndWriteOnlyDepthBuffer, ColorBufferAndReadableDepthBuffer};
    enum class BasicClearMode : std::uint32_t { 
        None = 0, ColorClear = 1, DepthClear = 2 };

    Framebuffer(unsigned int init_width = s_randomLen_,
        unsigned int init_height = s_randomLen_, BasicBufferType type =
        BasicBufferType::ColorBufferAndWriteOnlyDepthBuffer,
        int additionalBufferAmount = 0);
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& another) noexcept;
    Framebuffer& operator=(Framebuffer&& another) noexcept;
    ~Framebuffer();

    void Resize(unsigned int width, unsigned int height);

    unsigned int GetWidth() const { return width_; }
    unsigned int GetHeight() const { return height_; }
    float GetAspect() const { return static_cast<float>(width_) / height_; }
    unsigned int GetFramebuffer() const { return frameBuffer_; }
    unsigned int GetDepthBuffer() const { return depthBuffer_; }
    unsigned int GetTextureColorBuffer() const { return textureColorBuffer_; }
    unsigned int GetAddtionalBuffer(size_t index) const { 
        return additionalBuffers_[index]; 
    }
    bool NeedDepthTesting() const { return depthBuffer_ != 0; }
    void SetClearMode(std::initializer_list<BasicClearMode> modes) { 
        clearMode_ = 0;
        for (auto mode : modes)
            clearMode_ |= static_cast<decltype(clearMode_)>(mode);
        return;
    }
    void UseAsRenderTarget();
    static void RestoreDefaultRenderTarget();
private:
    unsigned int frameBuffer_ = 0;
    unsigned int depthBuffer_ = 0;
    unsigned int textureColorBuffer_ = 0;
    BasicBufferType basicBufferType_;
    std::vector<unsigned int> additionalBuffers_;

    unsigned int width_;
    unsigned int height_;
    static const unsigned int s_randomLen_ = 1000u;
    // Note: this will be reset to all enabled in the ctor.
    typename std::underlying_type<BasicClearMode>::type clearMode_ = 0;

    void GenerateAndAttachWriteOnlyDepthBuffer_();
    void GenerateAndAttachReadableDepthBuffer_();
    void GenerateAndAttachTextureBuffer_();
    void GenerateAdditionalBuffers_(unsigned int begin);
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_