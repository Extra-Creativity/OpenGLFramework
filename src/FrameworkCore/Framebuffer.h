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
    bool needDepthClear = true;
    enum class BasicBufferType { OnlyColorBuffer, OnlyReadableDepthBuffer,
       ColorBufferAndWriteOnlyDepthBuffer, ColorBufferAndReadableDepthBuffer};

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

    size_t GetWidth() const { return width_; }
    size_t GetHeight() const { return height_; }
    unsigned int GetFramebuffer() const { return frameBuffer_; }
    unsigned int GetDepthBuffer() const { return depthBuffer_; }
    unsigned int GetTextureColorBuffer() const { return textureColorBuffer_; }
    bool NeedDepthTesting() const { return depthBuffer_ != 0; }
private:
    unsigned int frameBuffer_ = 0;
    unsigned int depthBuffer_ = 0;
    unsigned int textureColorBuffer_ = 0;
    BasicBufferType basicBufferType_;
    std::vector<unsigned int> additionalBuffers_;

    unsigned int width_;
    unsigned int height_;
    static const unsigned int s_randomLen_ = 1000u;

    void GenerateAndAttachWriteOnlyDepthBuffer_();
    void GenerateAndAttachReadableDepthBuffer_();
    void GenerateAndAttachTextureBuffer_();
    void GenerateAdditionalBuffers_(unsigned int begin);
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_