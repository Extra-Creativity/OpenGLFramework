#pragma once
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/Camera.h"
#include "../Base/AssetLoader.h"

class ShadowMap
{
    using FrameBuffer = OpenGLFramework::Core::Framebuffer;
public:
    ShadowMap(unsigned int init_width, unsigned int init_height,
        ExampleBase::AssetLoader& loader) : ShadowMap{ { init_width, init_height,
            FrameBuffer::GetDepthTextureDefaultParamConfig(), {} }, loader }
    {}

    static void Render(ShadowMap& shadowMap, 
        ExampleBase::AssetLoader::ModelContainer&);
    const glm::mat4& GetLightSpaceMat() { return lightSpaceMat_; }
    auto& GetLightSpaceCamera() { return lightSpaceCamera_; }
    virtual void ResizeBuffer(unsigned int width, unsigned int height)
    { 
        buffer_ = FrameBuffer{ width, height,
            decltype(buffer_)::GetDepthTextureDefaultParamConfig(), {} 
        };
    }

    auto GetAspect() { return buffer_.GetAspect(); }
    virtual unsigned int GetShadowBuffer() { return buffer_.GetDepthBuffer(); }
    std::pair<unsigned int, unsigned int> GetWidthAndHeight() { 
        return { buffer_.GetWidth(), buffer_.GetHeight() };
    }
protected:
    ShadowMap(FrameBuffer frameBuffer, ExampleBase::AssetLoader& loader) :
        buffer_{ std::move(frameBuffer) },
        shadowMapShader_{ loader.GetShader("shadow map") },
        lightSpaceCamera_{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} },
        lightSpaceMat_{}
    {
        lightSpaceCamera_.fov = 90;
    }

    FrameBuffer buffer_;
private:
    void SetShaderParams_();
    void UpdateLightSpaceMat_();
    void Render_(ExampleBase::AssetLoader::ModelContainer&);

    OpenGLFramework::Core::Shader& shadowMapShader_;
    OpenGLFramework::Core::Camera lightSpaceCamera_;
    glm::mat4 lightSpaceMat_;
};

class ShadowMapForVSSM : public ShadowMap
{
    using FrameBuffer = OpenGLFramework::Core::Framebuffer;
public:
    ShadowMapForVSSM(unsigned int init_width, unsigned int init_height,
        ExampleBase::AssetLoader& loader) : ShadowMap{
            { init_width, init_height}, loader
    } {}

    void ResizeBuffer(unsigned int width, unsigned int height) override
    {
        buffer_ = OpenGLFramework::Core::Framebuffer{ width, height };
    }

    unsigned int GetShadowBuffer() override { return buffer_.GetColorBuffer(); }
};