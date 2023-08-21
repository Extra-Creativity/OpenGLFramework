#pragma once
#ifndef OPENGLFRAMEWORK_CORE_TEXTURE_H_
#define OPENGLFRAMEWORK_CORE_TEXTURE_H_

#include <glad/glad.h>

#include "ConfigHelpers/TextureConfig.h"

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
private:
    friend CPUTextureData GetCPUDataFromAnyTexture(int, int, int,
        int, unsigned int, int);
    CPUTextureData(unsigned char* ptr, int init_width, int init_height,
        int init_channels) : texturePtr{ ptr }, width{ init_width },
        height{ init_height }, channels{ init_channels } {};
};

GLenum GetGPUChannelFromCPUChannel(int cpuChannel);

CPUTextureData GetCPUDataFromAnyTexture(int width, int height, int channel,
    int gpuBindTextureType, unsigned int bindTextureID, int gpuSubTextureType);

class Texture
{
    inline const static TextureParamConfig c_defaultConfig_ = {
        .minFilter = TextureParamConfig::MinFilterType::LinearAfterMIPMAPLinear,
        .needMIPMAP = true
    };
public:
    static const auto& GetDefaultParamConfig() { return c_defaultConfig_; }
    auto GetID() const { return ID_; }
    Texture(const std::filesystem::path& path, 
        const TextureParamConfig& config = c_defaultConfig_);
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& another) noexcept : ID_{ std::exchange(another.ID_, 0) },
        cpuChannel_{ std::exchange(another.cpuChannel_, 0) } {};
    Texture& operator=(Texture&& another) noexcept { 
        if (&another == this)
            return *this;

        glDeleteTextures(1, &ID_);
        ID_ = std::exchange(another.ID_, 0);
        cpuChannel_ = std::exchange(another.cpuChannel_, 0);

        return *this;
    };
    ~Texture() {
        glDeleteTextures(1, &ID_);
        return;
    };
    std::pair<int, int> GetWidthAndHeight() const;
    CPUTextureData GetCPUData() const;
private:
    unsigned int ID_;
    int cpuChannel_;
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_TEXTURE_H_
