#include "SkyboxTexture.h"
#include "ContextManager.h"
#include "MainWindow.h"
#include "../Utility/IO/IniFile.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include <filesystem>

using namespace OpenGLFramework::Core;
OpenGLFramework::IOExtension::IniFile config{ TEST_CONFIG_PATH };

TEST_CASE("Simple-Skybox")
{
    // each row is not aligned(6 col * 3 channels = 18 % 4 != 0), so unpack with 1.
    int initialAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &initialAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    auto& path = config.rootSection.GetEntry("simple_skybox")->get();
    REQUIRE(std::filesystem::exists(path));

    SkyBoxTexture tex{ path, SkyBoxTexture::TextureSegmentType::HorizontalLeft };
    const int r = std::stoi(config.rootSection.GetEntry("r")->get()),
              g = std::stoi(config.rootSection.GetEntry("g")->get()),
              b = std::stoi(config.rootSection.GetEntry("b")->get());

    for (int i = 0; i < 6; i++)
    {
        auto facet = tex.GetCPUData(i);
        size_t len = static_cast<size_t>(facet.width) * facet.height * facet.channels;
        for (size_t j = 0; j < len; j += 3)
        {
            REQUIRE(facet.texturePtr[j] == r);
            REQUIRE(facet.texturePtr[j + 1] == g);
            REQUIRE(facet.texturePtr[j + 2] == b);
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, initialAlignment);
}

int main()
{
    [[maybe_unused]] ContextManager& manager = ContextManager::GetInstance();
    MainWindow useForContextWindow{ 50, 50, "test", false };
    auto result = Catch::Session().run();
    return result;
}