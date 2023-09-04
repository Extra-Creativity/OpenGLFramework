#include "ShadowMap.h"

using namespace OpenGLFramework;

void ShadowMap::Render(ShadowMap& shadowMap, 
	const std::vector<OpenGLFramework::Core::BasicTriRenderModel>& scene)
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
    float near = 5.0f, far = 20.0f;
	float top = near * glm::tan(glm::radians(lightSpaceCamera_.fov / 2)),
		right = top * buffer_.GetAspect();
	glm::mat4 projection = glm::ortho(-right, right, -top, top, near, far);
    lightSpaceMat_ = projection * lightSpaceCamera_.GetViewMatrix();
}

void ShadowMap::Render_(
	const std::vector<OpenGLFramework::Core::BasicTriRenderModel>& models)
{
	using enum Core::Framebuffer::BasicClearMode;
	buffer_.Clear({ DepthClear }, false);

	for (auto& model : models)
	{
		shadowMapShader_.SetMat4("modelMat", model.transform.GetModelMatrix());
		model.Draw(shadowMapShader_, buffer_);
	}
}