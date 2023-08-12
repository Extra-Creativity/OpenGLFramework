#include "ScreenShader.h"

void ScreenShader::Render(ScreenShader& screenShader, ShadowMap& shadowMap,
	const int& shadowOption, ExampleBase::AssetLoader::ModelContainer& scene)
{
	screenShader.SetShaderParams_(shadowMap, shadowOption);
	screenShader.Render_(shadowMap, scene);
}

void ScreenShader::SetShaderParams_(ShadowMap& shadowMap, int shadowOption)
{
	float near = 0.1f, far = 100.0f;
	screenShader_.Activate();
	screenShader_.SetMat4("lightSpaceMat", shadowMap.GetLightSpaceMat());
	screenShader_.SetVec3("lightPos", 
		shadowMap.GetLightSpaceCamera().GetPosition());
	screenShader_.SetVec3("viewPos", camera_.GetPosition());
	screenShader_.SetMat4("view", camera_.GetViewMatrix());
	screenShader_.SetMat4("projection", glm::perspective(camera_.fov,
		shadowMap.GetAspect(), near, far));
	screenShader_.SetInt("shadowOption", shadowOption);
}

void ScreenShader::BindShadowMap_(ShadowMap& shadowMap, int textureBeginID,
	OpenGLFramework::Core::Shader& shader)
{
	glActiveTexture(GL_TEXTURE0 + textureBeginID);
	shader.SetInt("shadowMap", textureBeginID);
	glBindTexture(GL_TEXTURE_2D, shadowMap.GetShadowBuffer());
	if (shadowMap.NeedMIPMAP())
		glGenerateMipmap(GL_TEXTURE_2D);
}

void ScreenShader::Render_(ShadowMap& shadowMap, 
	ExampleBase::AssetLoader::ModelContainer& models)
{
	for (auto& [name, model] : models)
	{
		screenShader_.SetMat4("model", model.transform.GetModelMatrix());
		model.Draw(screenShader_, 
			std::bind_front(BindShadowMap_, std::ref(shadowMap)), nullptr);
	}
};
