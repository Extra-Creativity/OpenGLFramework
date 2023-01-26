#include "ContextManager.h"
#include "MainWindow.h"
#include "Model.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using namespace OpenGLFramework::Core;

TEST_CASE("Cube")
{
    // this will generate normals by assimp.
    SECTION("ModelWithoutNormal")
    {
        BasicTriModel model{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
                R"(\OpenGLFramework\Resources\Models\Cube\CubeWithoutNormal.obj)" };
        auto size = model.meshes.size();
        REQUIRE(1 == size);
        auto& mesh = model.meshes[0];
        REQUIRE_THAT(mesh.vertices, Catch::Matchers::UnorderedEquals(
            std::vector<glm::vec3>{
                { 0.0 , 0.0 , 0.0 },
                { 0.0 , 0.0 , 1.0 },
                { 0.0 , 1.0 , 0.0 },
                { 0.0 , 1.0 , 1.0 },
                { 1.0 , 0.0 , 0.0 },
                { 1.0 , 0.0 , 1.0 },
                { 1.0 , 1.0 , 0.0 },
                { 1.0 , 1.0 , 1.0 }
            })
        );
        REQUIRE_THAT(mesh.triangles, Catch::Matchers::UnorderedEquals(
            std::vector<glm::ivec3>{
                { 1, 7, 5},
                { 1, 3, 7},
                { 1, 4, 3},
                { 1, 2, 4},
                { 3, 8, 7},
                { 3, 4, 8},
                { 5, 7, 8},
                { 5, 8, 6},
                { 1, 5, 6},
                { 1, 6, 2},
                { 2, 6, 8},
                { 2, 8, 4}
            })
        );
    }
}

int main()
{
    ContextManager& manager = ContextManager::GetInstance();
    MainWindow useForContextWindow{0, 0, "test"};
    auto result = Catch::Session().run();
    return result;
}