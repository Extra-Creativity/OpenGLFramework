#pragma once
#include "FrameworkCore/Framebuffer.h"
#include "FrameworkCore/Camera.h"
#include "../Base/AssetLoader.h"

class ShadowMap
{
protected:
    using Handle_ = decltype([] {
        float border[] = { 1.0,1.0,1.0,1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    });

    using FrameBuffer = OpenGLFramework::Core::Framebuffer;
    using TextureParamConfig = OpenGLFramework::Core::TextureParamConfig;
    using TextureConfigCRef = OpenGLFramework::Core::TextureConfigCRef;
private:
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
        ExampleBase::AssetLoader& loader) : ShadowMap{ 
            { init_width, init_height, c_config_, {} 
        }, 
            loader
    } {}
    virtual ~ShadowMap() = default;

    static void Render(ShadowMap& shadowMap, 
        ExampleBase::AssetLoader::ModelContainer&);
    const glm::mat4& GetLightSpaceMat() { return lightSpaceMat_; }
    auto& GetLightSpaceCamera() { return lightSpaceCamera_; }
    const auto& GetLightSpaceCamera() const { return lightSpaceCamera_; }
    virtual void ResizeBuffer(unsigned int width, unsigned int height)
    { 
        buffer_ = FrameBuffer{ width, height, c_config_, {} };
    }

    auto GetAspect() const { return buffer_.GetAspect(); }
    virtual unsigned int GetShadowBuffer() const { return buffer_.GetDepthBuffer(); }
    std::pair<unsigned int, unsigned int> GetWidthAndHeight() const { 
        return { buffer_.GetWidth(), buffer_.GetHeight() };
    }
    virtual bool NeedMIPMAP() const { return false; }
protected:
    ShadowMap(FrameBuffer frameBuffer, ExampleBase::AssetLoader& loader,
        std::string_view shaderName = "shadow map") :
        buffer_{ std::move(frameBuffer) },
        shadowMapShader_{ loader.GetShader(shaderName) },
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
    using TextureParamConfig = OpenGLFramework::Core::TextureParamConfig;
    inline static const TextureParamConfig c_param_config_ = {
        .minFilter = TextureParamConfig::MinFilterType::LinearAfterMIPMAPLinear,
        .wrapS = TextureParamConfig::WrapType::ClampToBorder,
        .wrapT = TextureParamConfig::WrapType::ClampToBorder,
        .wrapR = TextureParamConfig::WrapType::ClampToBorder,
        .auxHandle = Handle_{}
    };

    inline static TextureConfigCRef c_config_{
        FrameBuffer::GetColorTextureDefaultConfig().first, c_param_config_
    };

public:
    ShadowMapForVSSM(unsigned int init_width, unsigned int init_height,
        ExampleBase::AssetLoader& loader) : ShadowMap{
            { init_width, init_height,
              FrameBuffer::GetDepthRenderBufferDefaultConfig(), { c_config_ }
            }, loader, "shadow map for vssm"
    } {}

    void ResizeBuffer(unsigned int width, unsigned int height) override
    {
        buffer_ = OpenGLFramework::Core::Framebuffer{ width, height,
            FrameBuffer::GetDepthRenderBufferDefaultConfig(), { c_config_ }
        };
    }

    unsigned int GetShadowBuffer() const override { return buffer_.GetColorBuffer(); }
    bool NeedMIPMAP() const override { return true; }
};