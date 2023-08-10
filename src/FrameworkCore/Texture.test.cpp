#include "Texture.h"
#include "ContextManager.h"
#include "MainWindow.h"
#include "../Utility/IO/IniFile.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include <cstring>

using namespace OpenGLFramework::Core;
OpenGLFramework::IOExtension::IniFile config{ TEST_CONFIG_PATH };

TEST_CASE("RoundTrip")
{
    auto& path = config.rootSection.GetEntry("texture_path")->get();
    std::filesystem::path unicodePath{
        reinterpret_cast<const char8_t*>(path.c_str()) 
    };

    REQUIRE(std::filesystem::exists(unicodePath));

    CPUTextureData cpuTex{ unicodePath };
    Texture tex{ unicodePath };

    auto [width, height] = tex.GetWidthAndHeight();
    REQUIRE(width == cpuTex.width);
    REQUIRE(height == cpuTex.height);

    auto roundTripCPUData = tex.GetCPUData();
    REQUIRE(roundTripCPUData.width == cpuTex.width);
    REQUIRE(roundTripCPUData.height == cpuTex.height);
    REQUIRE(roundTripCPUData.channels == cpuTex.channels);
    REQUIRE(std::memcmp(cpuTex.texturePtr, roundTripCPUData.texturePtr, 
        roundTripCPUData.width * roundTripCPUData.height * roundTripCPUData.channels) == 0);
}

int main()
{
    [[maybe_unused]] ContextManager& manager = ContextManager::GetInstance();
    MainWindow useForContextWindow{ 50, 50, "test", false };
    auto result = Catch::Session().run();
    return result;
}