#pragma once
#ifndef OPENGLFRAMEWORK_CORE_TEXTURE_H_
#define OPENGLFRAMEWORK_CORE_TEXTURE_H_

#include <glad/glad.h>

#include <string>
#include <filesystem>

namespace OpenGLFramework::Core
{
    
struct CPUTextureData
{
    unsigned char* texturePtr;
    int width;
    int height;
    int channels;
    CPUTextureData(const std::filesystem::path& path);
    CPUTextureData(const CPUTextureData&) = delete;
    CPUTextureData& operator=(const CPUTextureData&) = delete;
    CPUTextureData(CPUTextureData&&) noexcept;
    CPUTextureData& operator=(CPUTextureData&&) noexcept;
    ~CPUTextureData();
};

GLenum GetGPUChannelFromCPUChannel(int cpuChannel);

class Texture
{
public:
    unsigned int ID;
    Texture(const std::filesystem::path& path);
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& another) noexcept : ID(another.ID) { another.ID = 0; };
    Texture& operator=(Texture&& another) noexcept { 
        if (&another == this)
            return *this;

        glDeleteTextures(1, &ID);
        ID = std::exchange(another.ID, 0);
        return *this;
    };
    ~Texture() {
        glDeleteTextures(1, &ID);
        return;
    };
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_TEXTURE_H_
