#include "Texture.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <array>
#include <iostream>

using namespace OpenGLFramework::Core;

// TODO : This test case is too ugly because it directly grasp implementation.
// Change it in the future.
TEST_CASE("Segment")
{
    CPUTextureData textureData{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL\)"
        R"(OpenGLFramework\Resources\Models\Skybox\test.png)"};
    int segmentWidth = textureData.width / 4,
        segmentHeight = textureData.height / 3,
        channelNum = textureData.channels;

    std::vector<unsigned char> segment(segmentWidth *
        segmentHeight * channelNum);
    unsigned char* segmentRawPtr = segment.data();

    const int segmentNum = 6;
    std::array<std::pair<int, int>, segmentNum> arr{
        std::pair{1, 2}, { 1, 0 }, { 0, 1 }, { 2, 1 }, { 1, 3 }, { 1, 1 } };

    GLenum gpuChannel = GetGPUChannelFromCPUChannel(textureData.channels);
    for (int i = 0; i < segmentNum; i++)
    {
        int beginRow = segmentHeight * arr[i].first,
            beginCol = segmentWidth * arr[i].second;

        for (int row = 0; row < segmentHeight; row++) {
            // This can be simplified in C++23 by channel_num reverse_copy.
            for (int col = 0; col < segmentWidth; col++) {
                auto newIdx = ((row + 1) * segmentWidth - col - 1) * channelNum,
                    oldIdx = ((row + beginRow) * textureData.width + beginCol + col) * channelNum;
                for (int k = 0; k < channelNum; k++)
                    segment[newIdx + k] = textureData.texturePtr[oldIdx + k];
            }

            // Not correct since skybox needs to be horizontally flip.
            //memcpy(segmentRawPtr + row * segmentWidth * channelNum, 
            //    textureData.texturePtr + channelNum * 
            //    ((row + beginRow) * textureData.width + beginCol),
            //    segmentWidth * channelNum);
        }
        for (int i = 0; i < segment.size(); i += 3) {
            REQUIRE(segment[i] == 78);
            REQUIRE(segment[i + 1] == 33);
            REQUIRE(segment[i + 2] == 20);
        }
        std::ranges::fill(segment, 0);
    }
    return;
}