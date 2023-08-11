#pragma once
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/Camera.h"
#include "../Base/AssetLoader.h"

class ShadowMap
{
public:
    OpenGLFramework::Core::Framebuffer buffer;

    ShadowMap(unsigned int init_width, unsigned int init_height, 
        ExampleBase::AssetLoader& loader) : buffer{ init_width, init_height,
        decltype(buffer)::GetDepthTextureDefaultParamConfig(), {} },
        shadowMapShader_{ loader.GetShader("shadow map") },
        lightSpaceCamera_{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} },
        lightSpaceMat_{}
    {
        lightSpaceCamera_.fov = 90;
    }

    static void Render(ShadowMap& shadowMap, 
        ExampleBase::AssetLoader::ModelContainer&);
    const glm::mat4& GetLightSpaceMat() { return lightSpaceMat_; }
    auto& GetLightSpaceCamera() { return lightSpaceCamera_; }
    static void ResizeBuffer(decltype(buffer)& frameBuffer,
        unsigned int width, unsigned int height)
    { 
        frameBuffer = OpenGLFramework::Core::Framebuffer{ width, height,
            decltype(buffer)::GetDepthTextureDefaultParamConfig(), {} 
        };
    }

private:
    void SetShaderParams_();
    void UpdateLightSpaceMat_();
    void Render_(ExampleBase::AssetLoader::ModelContainer&);

    OpenGLFramework::Core::Shader& shadowMapShader_;
    OpenGLFramework::Core::Camera lightSpaceCamera_;
    glm::mat4 lightSpaceMat_;
};