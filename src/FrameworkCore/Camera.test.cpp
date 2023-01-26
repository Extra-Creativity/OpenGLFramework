#include "Camera.h"

#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>

#include <numbers>

using namespace OpenGLFramework::Core;

static float tolerantEpsilon = 1e-5f;

TEST_CASE("Simple")
{
    Camera camera{ glm::vec3{1, 1, 1}, glm::vec3{0, 1, 0}, glm::vec3{0, 0, 1} };
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.GetPosition(), glm::vec3{1, 1, 1}, 
            tolerantEpsilon) ));
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Up(), glm::vec3{ 0, 1, 0 }, tolerantEpsilon)
    ));

    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Down(), glm::vec3{ 0, -1, 0 }, tolerantEpsilon)
    ));

    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Left(), glm::vec3{ 1, 0, 0 }, tolerantEpsilon)
    ));

    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Right(), glm::vec3{ -1, 0, 0 }, tolerantEpsilon)
    ));

    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Front(), glm::vec3{ 0, 0, 1 }, tolerantEpsilon)
    ));

    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Back(), glm::vec3{ 0, 0, -1 }, tolerantEpsilon)
    ));

    camera.Translate(glm::vec3{ -1, 0.5, 0.1 });
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.GetPosition(), glm::vec3{0, 1.5, 1.1},
            tolerantEpsilon) ));

    camera.Rotate(45, glm::vec3{0, 1, 0});
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Front(), glm::vec3{ std::numbers::sqrt2 / 2, 0,
            std::numbers::sqrt2 / 2 }, tolerantEpsilon) ));
    
    camera.Rotate(glm::quat{ std::numbers::sqrt2 / 2 , 0, 0, 
         std::numbers::sqrt2 / 2 });
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Front(), glm::vec3{ 0, std::numbers::sqrt2 / 2,
            std::numbers::sqrt2 / 2 }, tolerantEpsilon)));

}

TEST_CASE("InitialNormalize")
{
    glm::vec3 front = { 0, 0, 2 }, up = { 1, 1, 1 };
    Camera camera{ glm::vec3{0,0,0}, up, front};
    INFO(camera.Up().x << camera.Up().y << camera.Up().z);
    REQUIRE(glm::all(
        glm::epsilonEqual(camera.Up(), { std::numbers::sqrt2 / 2, 
            std::numbers::sqrt2 / 2, 0}, tolerantEpsilon)
    ));
}