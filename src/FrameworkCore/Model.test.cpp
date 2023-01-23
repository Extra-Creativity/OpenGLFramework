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
        Model model{ R"(D:\111\University\Course\CS\Computer Graphics\OpenGL)"
                R"(\OpenGLFramework\Resources\Models\Cube\CubeWithoutNormal)" };
        REQUIRE(model.meshes.size() == 1);
    }
}

int main()
{
    InitContext();
    auto result = Catch::Session().run();
    EndContext();
    return result;
}