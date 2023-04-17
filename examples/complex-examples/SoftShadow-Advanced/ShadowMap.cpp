#include "ShadowMap.h"

using namespace OpenGLFramework;

void ShadowMap::Render(ShadowMap& shadowMap, 
    ExampleBase::AssetLoader::ModelContainer& scene)
{
    shadowMap.UpdateLightSpaceMat_();
    shadowMap.SetShaderParams_();
    shadowMap.Render_(scene);
}

void ShadowMap::SetShaderParams_()
{
    shadowMapShader_.Activate();
    shadowMapShader_.SetMat4("lightSpaceMat", lightSpaceMat_);
}

void ShadowMap::UpdateLightSpaceMat_()
{
    float near = 0.1f, far = 100.0f;
    glm::mat4 projection = glm::perspective(
        glm::radians(lightSpaceCamera_.fov), buffer.GetAspect(), near, far);
    lightSpaceMat_ = projection * lightSpaceCamera_.GetViewMatrix();
}

void ShadowMap::Render_(ExampleBase::AssetLoader::ModelContainer& models)
{
	using enum Core::Framebuffer::BasicClearMode;
	auto& firstModel = models.begin()->second;
	shadowMapShader_.SetMat4("modelMat", firstModel.transform.GetModelMatrix());
	buffer.SetClearMode({ DepthClear });
	firstModel.Draw(shadowMapShader_, buffer);

	for (auto& [name, model] : models | std::views::drop(1))
	{
		shadowMapShader_.SetMat4("modelMat", model.transform.GetModelMatrix());
		buffer.SetClearMode({ None });
		model.Draw(shadowMapShader_, buffer);
	}
}