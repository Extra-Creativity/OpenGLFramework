#pragma once
#include "../Base/AssetLoader.h"
#include "FrameworkCore/Camera.h"
#include "ShadowMap.h"

class ScreenShader
{
public:
    ScreenShader(ExampleBase::AssetLoader& loader) :
        screenShader_{ loader.GetShader("screen") },
        camera_{ {-30, 10, 18}, {0, 1, 0}, {30, 0, -18} }
    {}

    static void Render(ScreenShader& screenShader, ShadowMap& shadowMap, 
       int& shadowOption, ExampleBase::AssetLoader::ModelContainer&);
    auto& GetCamera() { return camera_; }
private:
    void SetShaderParams_(ShadowMap& shadowMap, int shadowOption);
    static void BindShadowMap_(ShadowMap& shadowMap, int textureBeginID, 
        OpenGLFramework::Core::Shader&);
    void Render_(ShadowMap&, ExampleBase::AssetLoader::ModelContainer&);

    OpenGLFramework::Core::Shader& screenShader_;
    OpenGLFramework::Core::Camera camera_;
};