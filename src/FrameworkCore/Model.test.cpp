#include "ContextManager.h"
#include "MainWindow.h"
#include "Model.h"
#include "SpecialModels/SpecialModel.h"
#include "../Utility/IO/IniFile.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <algorithm>
#include <filesystem>

using namespace OpenGLFramework::Core;
OpenGLFramework::IOExtension::IniFile config{ TEST_CONFIG_PATH };

TEST_CASE("Cube")
{
    auto path = config.rootSection.GetEntry("Cube_Model");
    REQUIRE((path.has_value() && std::filesystem::exists(path->get())));
    BasicTriModel model{ path->get() };
    auto realCube = Cube::GetBasicTriMesh();

    SECTION("Model")
    {
        SECTION("Size")
        {
            auto size = model.meshes.size();
            REQUIRE(1 == size);

            auto& mesh = model.meshes[0];
            REQUIRE(mesh.triangles.size() == realCube.triangles.size());
        }

        auto& mesh = model.meshes[0];
        SECTION("Vertices")
        {
            REQUIRE_THAT(mesh.vertices, Catch::Matchers::UnorderedEquals(realCube.vertices));
        }

        SECTION("Triangles")
        {
            std::vector<bool> exists(mesh.triangles.size());
            for (auto& tri : mesh.triangles)
            {
                auto verts = mesh.GetTriangleVerts(tri);
                auto it = std::ranges::find_if(realCube.triangles,
                    [&realCube, &verts](auto& realTri) {
                        auto realVerts = realCube.GetTriangleVerts(realTri);
                        return std::ranges::is_permutation(verts, realVerts,
                            [](auto refv1, auto refv2) {
                            return refv1.get() == refv2.get();
                        });
                    });
                REQUIRE(it != realCube.triangles.end());
                auto idx = it - realCube.triangles.begin();
                REQUIRE(!exists[idx]);
                exists[idx] = true;
            }
        }
    }
}