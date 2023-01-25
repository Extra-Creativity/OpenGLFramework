#include "PrepareContext.h"
#include "Model.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using namespace OpenGLFramework::Core;

TEST_CASE("Cube")
{
    std::vector<glm::vec3> vertices;
    SECTION("WithoutNormal")
    {
        BasicTriModel model{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
                R"(\OpenGLFramework\Resources\Models\Cube\CubeWithoutNormal.obj)" };
        auto size = model.meshes.size();
        REQUIRE(1 == size);
        auto& mesh = model.meshes[0];
        for (auto i = 0; i < mesh.vertices.size(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                std::cout << mesh.vertices[i][j] << " ";
            }
            std::cout << "\n";
        }
        REQUIRE_THAT(mesh.vertices, Catch::Matchers::UnorderedEquals(std::vector<glm::vec3>{
            { 0.0 , 0.0 , 0.0},
            { 0.0 , 0.0 , 1.0 },
            { 0.0 , 1.0 , 0.0 },
            { 0.0 , 1.0 , 1.0 },
            { 1.0 , 0.0 , 0.0 },
            { 1.0 , 0.0 , 1.0 },
            { 1.0 , 1.0 , 0.0 },
            { 1.0 , 1.0 , 1.0 }
        }));
    }
}

int main()
{
    InitContext();
    auto result = Catch::Session().run();
    EndContext();
    return result;
}