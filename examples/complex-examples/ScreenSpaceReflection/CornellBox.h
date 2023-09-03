#pragma once
#include "FrameworkCore/Model.h"
#include "FrameworkCore/Shader.h"
#include "FrameworkCore/Camera.h"
#include <vector>

class CornellBox
{
public:
    CornellBox(OpenGLFramework::Core::Shader&);
    void Draw(float aspect, float near, float far,
        OpenGLFramework::Core::Camera& camera);
private:
    std::vector<OpenGLFramework::Core::BasicTriRenderModel> models_;
    OpenGLFramework::Core::Shader& directShader_;
};
