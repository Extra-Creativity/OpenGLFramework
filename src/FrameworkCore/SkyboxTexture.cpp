#include "SkyboxTexture.h"

#include <cassert>

#include "stb_image.h"

namespace OpenGLFramework::Core
{

SkyBoxTexture::SkyBoxTexture(const std::filesystem::path& path,
    TextureSegmentType type)
{
    GenerateSkyBox_();
    // TODO: In the future, we'll make it more structual.
    assert(type == TextureSegmentType::HorizontalLeft);
    CPUTextureData textureData{ path };

    int segmentWidth = textureData.width / 4, 
        segmentHeight = textureData.height / 3,
        channelNum = textureData.channels;
    
    std::vector<unsigned char> segment(segmentWidth *
        segmentHeight * channelNum);
    unsigned char* segmentRawPtr = segment.data();

    const int segmentNum = 6;
    std::array<std::pair<int, int>, segmentNum> arr{ 
        std::pair{1, 2}, { 1, 0 }, { 0, 1 }, { 2, 1 }, { 1, 3 }, { 1, 1 } };
    std::array<decltype(&HorizontalFlipSegment_), segmentNum> handles{
        &HorizontalFlipSegment_, &HorizontalFlipSegment_,
        &VerticalFlipSegment_, &VerticalFlipSegment_,
        &HorizontalFlipSegment_,&HorizontalFlipSegment_
    };

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(textureData.channels);
    for (int i = 0; i < segmentNum; i++)
    {
        std::invoke(handles[i], segmentRawPtr, segmentWidth, segmentHeight, 
            textureData, arr[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gpuChannel,
            segmentWidth, segmentHeight, 0, gpuChannel,
            GL_UNSIGNED_BYTE, segmentRawPtr);
    }
    AttachSkyBoxAttributes_();
    return;
};

SkyBoxTexture::SkyBoxTexture(const std::filesystem::path& path, 
    const std::array<std::string_view, c_skyboxFacetNum_>& append)
{
    GenerateSkyBox_();
    auto root = path.parent_path() / path.stem();
    auto extension = path.extension().native();
    
    const int lastID = c_skyboxFacetNum_ - 1;
    for (int i = 0; i < lastID; i++)
        AttachFacetTexture_(i, 
            std::filesystem::path{ root }.concat(append[i]).concat(extension));
    // To reduce overhead.
    AttachFacetTexture_(lastID, 
        root.concat(append[lastID]).concat(std::move(extension)));
    AttachSkyBoxAttributes_();
    return;
};

SkyBoxTexture::SkyBoxTexture(const std::array<std::filesystem::path, 
    c_skyboxFacetNum_>& texturePaths)
{
    GenerateSkyBox_();

    for (int i = 0; i < c_skyboxFacetNum_; i++)
        AttachFacetTexture_(i, texturePaths[i]);
    AttachSkyBoxAttributes_();
    return;
};

void SkyBoxTexture::GenerateSkyBox_()
{
    glGenTextures(1, &skyboxID_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID_);
    return;
}

void SkyBoxTexture::AttachFacetTexture_(int facetID, const std::filesystem::path& path)
{
    CPUTextureData textureData{ path };
    GLenum gpuChannel = GetGPUChannelFromCPUChannel(textureData.channels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + facetID, 0, gpuChannel,
        textureData.width, textureData.height, 0, gpuChannel,
        GL_UNSIGNED_BYTE, textureData.texturePtr);
    return;
}

void SkyBoxTexture::AttachSkyBoxAttributes_()
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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