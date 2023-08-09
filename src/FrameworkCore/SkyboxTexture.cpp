#include "SkyboxTexture.h"

#include <cassert>

#include "stb_image.h"

namespace OpenGLFramework::Core
{

SkyBoxTexture::SkyBoxTexture(const std::filesystem::path& path,
    TextureSegmentType type, const TextureParamConfig& config)
{
    GenerateAndBindSkyBox_();
    AttachAllInOneTexture_(path, type);
    config.Apply();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return;
};

SkyBoxTexture::SkyBoxTexture(const std::filesystem::path& path, 
    const std::array<std::string_view, c_skyboxFacetNum_>& append,
    const TextureParamConfig& config)
{
    GenerateAndBindSkyBox_();
    auto root = path.parent_path() / path.stem();
    auto extension = path.extension().native();
    
    const int lastID = c_skyboxFacetNum_ - 1;
    for (int i = 0; i < lastID; i++)
        AttachFacetTexture_(i, 
            std::filesystem::path{ root }.concat(append[i]).concat(extension));
    // To reduce overhead.
    AttachFacetTexture_(lastID, 
        root.concat(append[lastID]).concat(std::move(extension)));
    config.Apply();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return;
};

SkyBoxTexture::SkyBoxTexture(const std::array<std::filesystem::path, 
    c_skyboxFacetNum_>& texturePaths, const TextureParamConfig& config)
{
    GenerateAndBindSkyBox_();
    for (int i = 0; i < c_skyboxFacetNum_; i++)
        AttachFacetTexture_(i, texturePaths[i]);
    config.Apply();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return;
};

SkyBoxTexture::SkyBoxTexture(SkyBoxTexture&& another) noexcept :
    skyboxID_{ std::exchange(another.skyboxID_, 0) }, 
    cpuChannel_{ std::exchange(another.cpuChannel_, 0) }
{};

SkyBoxTexture& SkyBoxTexture::operator=(SkyBoxTexture&& another) noexcept
{
    ReleaseResources_();
    skyboxID_ = std::exchange(another.skyboxID_, 0);
    cpuChannel_ = std::exchange(another.cpuChannel_, 0);
    return *this;
}

SkyBoxTexture::~SkyBoxTexture()
{
    ReleaseResources_();
    return;
}

CPUTextureData SkyBoxTexture::GetCPUData(int idx)
{
    int width = 0, height = 0;
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID_);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
        GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
        GL_TEXTURE_HEIGHT, &height);

    return GetCPUDataFromAnyTexture(width, height, cpuChannel_, 
        GL_TEXTURE_CUBE_MAP, skyboxID_, GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx);
};


void SkyBoxTexture::ReleaseResources_()
{
    glDeleteTextures(1, &skyboxID_);
    return;
};

void SkyBoxTexture::GenerateAndBindSkyBox_()
{
    glGenTextures(1, &skyboxID_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID_);
    return;
}

void SkyBoxTexture::AttachAllInOneTexture_(const std::filesystem::path& path,
    TextureSegmentType type)
{
    CPUTextureData textureData{ path };
    cpuChannel_ = textureData.channels;

    int segmentWidth = textureData.width / 4,
        segmentHeight = textureData.height / 3,
        channelNum = textureData.channels;

    std::vector<unsigned char> segment(segmentWidth *
        segmentHeight * channelNum);
    unsigned char* segmentRawPtr = segment.data();

    assert(type == TextureSegmentType::HorizontalLeft);
    // These two arrays are only for HorizontalLeft.
    std::array<std::pair<int, int>, c_segmentNum_> arr{
        std::pair{1, 2}, { 1, 0 }, { 0, 1 }, { 2, 1 }, { 1, 3 }, { 1, 1 } };
    std::array<decltype(&HorizontalFlipSegment_), c_segmentNum_> handles{
        &HorizontalFlipSegment_, &HorizontalFlipSegment_,
        &VerticalFlipSegment_, &VerticalFlipSegment_,
        &HorizontalFlipSegment_,&HorizontalFlipSegment_
    };

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(textureData.channels);
    TextureGenConfig genConfig = GetDefaultTextureGenConfig(gpuChannel);

    for (int i = 0; i < c_segmentNum_; i++)
    {
        std::invoke(handles[i], segmentRawPtr, segmentWidth, segmentHeight,
            textureData, arr[i]);
        genConfig.Apply(static_cast<TextureType>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i),
            segmentWidth, segmentHeight, segmentRawPtr);
    }
}

void SkyBoxTexture::AttachFacetTexture_(int facetID, const std::filesystem::path& path)
{
    CPUTextureData textureData{ path };
    cpuChannel_ = textureData.channels;

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(textureData.channels);
    auto genConfig = GetDefaultTextureGenConfig(gpuChannel);
    genConfig.Apply(static_cast<TextureType>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + facetID),
        textureData);
    return;
}

void SkyBoxTexture::HorizontalFlipSegment_(unsigned char* segmentPtr,
    int segmentWidth, int segmentHeight, CPUTextureData& textureData,
    std::pair<int, int>& segPos)
{
    int beginRow = segmentHeight * segPos.first,
        beginCol = segmentWidth * segPos.second,
        channelNum = textureData.channels;

    for (int row = 0; row < segmentHeight; row++) {
        // This can be simplified in C++23 by channel_num reverse_copy.
        for (int col = 0; col < segmentWidth; col++) {
            auto newIdx = ((row + 1) * segmentWidth - col - 1) * channelNum,
                oldIdx = ((row + beginRow) * textureData.width + beginCol + col) * channelNum;
            for (int k = 0; k < channelNum; k++)
                segmentPtr[newIdx + k] = textureData.texturePtr[oldIdx + k];
        }
    }
    return;
};

void SkyBoxTexture::VerticalFlipSegment_(unsigned char* segmentPtr,
    int segmentWidth, int segmentHeight, CPUTextureData& textureData,
    std::pair<int, int>& segPos)
{
    int beginRow = segmentHeight * segPos.first,
        beginCol = segmentWidth * segPos.second,
        channelNum = textureData.channels;

    for (int row = 0; row < segmentHeight; row++) {
        memcpy(segmentPtr + (segmentHeight - row - 1) * segmentWidth
            * channelNum, textureData.texturePtr + channelNum *
            ((row + beginRow) * textureData.width + beginCol),
            segmentWidth * channelNum);
    }
    return;
};

}