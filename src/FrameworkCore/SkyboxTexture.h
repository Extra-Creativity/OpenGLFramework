#pragma once
#ifndef OPENGLFRAMEWORK_CORE_SKYBOXTEXTURE_H_
#define OPENGLFRAMEWORK_CORE_SKYBOXTEXTURE_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <array>

#include "Texture.h"

namespace OpenGLFramework::Core
{

class SkyBoxTexture
{
    static constexpr int c_skyboxFacetNum_ = 6;
    static constexpr int c_segmentNum = 6;
public:
    enum class TextureSegmentType {
        HorizontalLeft
    };

    SkyBoxTexture(const std::filesystem::path& path, TextureSegmentType type);
    SkyBoxTexture(const std::filesystem::path& path, 
        const std::array<std::string_view, c_skyboxFacetNum_>& append = {
            "_px", "_nx", "_py", "_ny", "_pz", "_nz"
        });
    SkyBoxTexture(const std::array<std::filesystem::path, 
                            c_skyboxFacetNum_>& texturePaths);
    SkyBoxTexture(const SkyBoxTexture&) = delete;
    SkyBoxTexture& operator=(const SkyBoxTexture&) = delete;
    SkyBoxTexture(SkyBoxTexture&&) = default;
    SkyBoxTexture& operator=(SkyBoxTexture&&) = default;
    unsigned int GetID() const { return skyboxID_; }
private:
    unsigned int skyboxID_;
    void GenerateSkyBox_();
    static void HorizontalFlipSegment_(unsigned char* segmentPtr, int segmentWidth,
        int segmentHeight, CPUTextureData& data, std::pair<int, int>& segPos);
    static void VerticalFlipSegment_(unsigned char* segmentPtr, int segmentWidth,
        int segmentHeight, CPUTextureData& data, std::pair<int, int>& segPos);
    void AttachFacetTexture_(int facetID, const std::filesystem::path& path);
    void AttachSkyBoxAttributes_();
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_SKYBOXTEXTURE_H_
