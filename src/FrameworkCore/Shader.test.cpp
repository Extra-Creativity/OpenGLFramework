#include "ContextManager.h"
#include "MainWindow.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "../Utility/IO/IniFile.h"

#include <glm/glm.hpp>

#include <iostream>
#include <filesystem>

using namespace OpenGLFramework::Core;
OpenGLFramework::IOExtension::IniFile config{ TEST_CONFIG_PATH };

void SetMVP(float width, float height, float near, float far,
    BasicTriRenderModel& model, Camera& camera, Shader& shader)
{
    auto modelMat = model.transform.GetModelMatrix();
    shader.SetMat4("model", modelMat);

    auto viewMat = camera.GetViewMatrix();
    shader.SetMat4("view", viewMat);

    auto projectionMat = glm::perspective(glm::radians(camera.fov),
        width / height, near, far);
    shader.SetMat4("projection", projectionMat);

    return;
}

int main()
{
    [[maybe_unused]] ContextManager& manager = ContextManager::GetInstance();
    MainWindow window{ 800, 600, "Shader Test" };

    BasicTriRenderModel model{ config.rootSection.GetEntry("Cube_Model")->get() };

    Shader shader{ config.rootSection.GetEntry("Vert_Shader")->get(),
        config.rootSection.GetEntry("Frag_Shader")->get() };
    Camera camera{ { 0.5, 0.7, 5}, {0, 0.8, 0.2}, {0, 0, -1} };
    camera.RotateAroundCenter(60, glm::vec3{ 0, 1, 0 }, {0, 0, 0});
    
    auto pos = camera.GetPosition();
    std::cout << pos.x << " " << pos.y << " " << pos.z << "\n";

    window.Register([&window, &model, &shader, &camera]() {
        shader.Activate();

        const auto [width, height] = window.GetWidthAndHeight();
        SetMVP(static_cast<float>(width), static_cast<float>(height), 
                4, 10, model, camera, shader);
        model.Draw(shader);
        return;
    });
    window.MainLoop(glm::vec4{0, 0, 0, 1});
    return 0;
}