#pragma once
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/Camera.h"
#include "../Base/AssetLoader.h"

class ShadowMap
{
private:
    using Handle_ = decltype([] {
        float border[] = { 1.0,1.0,1.0,1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    });

    using FrameBuffer = OpenGLFramework::Core::Framebuffer;
    using TextureParamConfig = OpenGLFramework::Core::TextureParamConfig;
    using TextureConfigCRef = OpenGLFramework::Core::TextureConfigCRef;

    inline static const TextureParamConfig c_param_config_ = {
        .minFilter = TextureParamConfig::MinFilterType::Linear,
        .wrapS = TextureParamConfig::WrapType::ClampToBorder,
        .wrapT = TextureParamConfig::WrapType::ClampToBorder,
        .wrapR = TextureParamConfig::WrapType::ClampToBorder,
        .auxHandle = Handle_{}
    };

    inline static TextureConfigCRef c_config_{
        FrameBuffer::GetDepthTextureDefaultConfig().first, c_param_config_
    };

public:
    ShadowMap(unsigned int init_width, unsigned int init_height,
        ExampleBase::AssetLoader& loader) : 
        buffer_{ init_width, init_height, c_config_, {}},
        shadowMapShader_{ loader.GetShader("shadow map") },
        lightSpaceCamera_{ {0, 4, 0}, {0, 0, 1}, {0, -1, 0} },
        lightSpaceMat_{} 
    {
        lightSpaceCamera_.fov = 135;
    }

    static void Render(ShadowMap& shadowMap,
        const std::vector<OpenGLFramework::Core::BasicTriRenderModel>& models);
    const glm::mat4& GetLightSpaceMat() { return lightSpaceMat_; }
    auto& GetLightSpaceCamera() { return lightSpaceCamera_; }
    const auto& GetLightSpaceCamera() const { return lightSpaceCamera_; }
    void ResizeBuffer(unsigned int width, unsigned int height)
    {
        buffer_ = FrameBuffer{ width, height, c_config_, {} };
    }

    auto GetAspect() const { return buffer_.GetAspect(); }
    unsigned int GetShadowBuffer() const { return buffer_.GetDepthBuffer(); }
    std::pair<unsigned int, unsigned int> GetWidthAndHeight() const {
        return { buffer_.GetWidth(), buffer_.GetHeight() };
    }

private:
    void SetShaderParams_();
    void UpdateLightSpaceMat_();
    void Render_(const std::vector<OpenGLFramework::Core::BasicTriRenderModel>& models);

    FrameBuffer buffer_;
    OpenGLFramework::Core::Shader& shadowMapShader_;
    OpenGLFramework::Core::Camera lightSpaceCamera_;
    glm::mat4 lightSpaceMat_;
};
