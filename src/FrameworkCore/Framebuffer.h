#pragma once
#ifndef OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_
#define OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_

#include <glm/glm.hpp>

namespace OpenGLFramework::Core
{

class Framebuffer
{
public:
    glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    
    Framebuffer(size_t init_width = s_randomLen_, 
        size_t init_height = s_randomLen_, bool needDepthTesting = true);
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& another) noexcept;
    Framebuffer& operator=(Framebuffer&& another) noexcept;
    ~Framebuffer();

    void Resize(size_t width, size_t height);

    size_t GetWidth() { return width_; }
    size_t GetHeight() { return height_; }
    int GetFramebuffer() { return frameBuffer_; }
    int GetTextureColorBuffer() { return textureColorBuffer_; }
    bool NeedDepthTesting() { return renderBuffer_ != 0; }
private:
    unsigned int frameBuffer_ = 0;
    unsigned int renderBuffer_ = 0;
    unsigned int textureColorBuffer_ = 0;

    size_t width_;
    size_t height_;
    // In C++23, this can be replaced by 1000z.
    static const size_t s_randomLen_ = static_cast<size_t>(1000);

    void GenerateAndAttachDepthBuffer_();
    void GenerateAndAttachTextureBuffer_();
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_