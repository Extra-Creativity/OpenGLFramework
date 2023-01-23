#pragma once
#ifndef OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_
#define OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_

#include <glm/glm.hpp>

namespace OpenGLFramework::Core
{

class Framebuffer
{
public:
    unsigned int frameBuffer = 0;
    unsigned int renderBuffer = 0;
    unsigned int textureColorBuffer = 0;
    glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    Framebuffer(size_t m_width = m_randomLen, size_t m_height = m_randomLen, bool needDepthTesting = true);
    Framebuffer(Framebuffer&) = delete;
    Framebuffer(Framebuffer&& another) noexcept : frameBuffer(another.frameBuffer), renderBuffer(another.renderBuffer),
        textureColorBuffer(another.textureColorBuffer), m_width(another.m_width), m_height(another.m_height) {
        another.frameBuffer = another.textureColorBuffer = another.renderBuffer = 0;
    }
    void resize(size_t m_width, size_t m_height);
    ~Framebuffer();

    size_t GetWidth() { return m_width; }
    size_t GetHeight() { return m_height; }
private:
    size_t m_width;
    size_t m_height;
    // In C++23, this can be replaced by 1000z.
    static const size_t m_randomLen = static_cast<size_t>(1000);
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_FRAMEBUFFER_H_