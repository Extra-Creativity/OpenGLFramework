#pragma once

#include "Texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <array>

namespace OpenGLFramework::Core
{

class SkyBoxTexture
{
    static constexpr int c_skyboxFacetNum_ = 6;
    static constexpr int c_segmentNum_ = 6;
    static constexpr inline TextureParamConfig c_defaultConfig_ = {
        .textureType = TextureType::CubeMap,
        .minFilter = TextureParamConfig::MinFilterType::Linear,
        .wrapS = TextureParamConfig::WrapType::ClampToEdge,
        .wrapT = TextureParamConfig::WrapType::ClampToEdge,
        .wrapR = TextureParamConfig::WrapType::ClampToEdge
    };
public:
    static const auto& GetDefaultParamConfig() { return c_defaultConfig_; }

    enum class TextureSegmentType {
        HorizontalLeft
    };

    SkyBoxTexture(const std::filesystem::path& path, TextureSegmentType type,
        const TextureParamConfig& config = c_defaultConfig_);
    SkyBoxTexture(const std::filesystem::path& path, 
        const std::array<std::string_view, c_skyboxFacetNum_>& append = {
            "_px", "_nx", "_py", "_ny", "_pz", "_nz"
        }, const TextureParamConfig& config = c_defaultConfig_);
    SkyBoxTexture(const std::array<std::filesystem::path, c_skyboxFacetNum_>& 
        texturePaths, const TextureParamConfig& config = c_defaultConfig_);
    SkyBoxTexture(const SkyBoxTexture&) = delete;
    SkyBoxTexture& operator=(const SkyBoxTexture&) = delete;
    SkyBoxTexture(SkyBoxTexture&&) noexcept;
    SkyBoxTexture& operator=(SkyBoxTexture&&) noexcept;
    ~SkyBoxTexture();
    unsigned int GetID() const { return skyboxID_; }
    CPUTextureData GetCPUData(int idx) const;
private:
    unsigned int skyboxID_;
    int cpuChannel_;
    void GenerateAndBindSkyBox_();
    static void HorizontalFlipSegment_(unsigned char* segmentPtr, int segmentWidth,
        int segmentHeight, CPUTextureData& data, std::pair<int, int>& segPos);
    static void VerticalFlipSegment_(unsigned char* segmentPtr, int segmentWidth,
        int segmentHeight, CPUTextureData& data, std::pair<int, int>& segPos);
    void AttachAllInOneTexture_(const std::filesystem::path& path,
        TextureSegmentType type);
    void AttachFacetTexture_(int facetID, const std::filesystem::path& path);
    void ReleaseResources_();
};

} // namespace OpenGLFramework::Core
