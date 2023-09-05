#include "FrameworkCore/Core_All.h"

#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"
#include "../Base/ImGUIHelper.h"
#include "CornellBox.h"
#include "ShadowMap.h"

int main()
{
	[[maybe_unused]] auto& manager = Core::ContextManager::GetInstance();
	ExampleBase::AssetLoader loader{
		std::filesystem::path{ CONFIG_DIR } / "config.ini",
		std::filesystem::path{ MODEL_DIR },
		std::filesystem::path{ SHADER_DIR }
	};
	auto& mainWindow = loader.GetMainWindowInstance();
	// Set up camera.
	Core::Camera camera{ {0, 0, 0}, {0, 1, 0}, {0, 0, -1} };
	camera.RotateAroundCenter(15, {0, 1, 0}, {0,0,0});
	camera.Translate(camera.Back() * 12.f);

	const auto [width, height] = mainWindow.GetWidthAndHeight();
	ShadowMap shadowMap{ width, height, loader };
	CornellBox box{ loader.GetShader("cornell") };
	
	Core::TextureGenConfig config{
		.gpuPixelFormat = GLHelper::ColorInternalFormat<GLHelper::ColorComponents::RGBA,
			GLHelper::GPUColorComponentSizeTag::All16,
			GLHelper::GPUComponentType::Float>::safe_value,
		.cpuPixelFormat = Core::TextureGenConfig::CPUPixelFormat::RGBA,
		.rawDataType = GLHelper::ToGLType<float>::safe_value
	};

	Core::Framebuffer gBuffer{ width, height, 
		Core::Framebuffer::GetDepthRenderBufferDefaultConfig(),
		{ 3, Core::TextureConfigCRef{ config, Core::Framebuffer::GetColorTextureDefaultConfig().second} }
	};

	mainWindow.Register([&] { ShadowMap::Render(shadowMap, box.GetModels()); });

	float near = 1.0f, far = 50.0f;
	mainWindow.Register([&]() {
		gBuffer.Clear();
		gBuffer.UseAsRenderTarget();
		box.Draw(mainWindow.GetAspect(), near, far, camera, shadowMap);
		Core::Framebuffer::RestoreDefaultRenderTarget();
	});

	auto quad = Core::Quad::GetBasicTriRenderModel();
	mainWindow.Register([&]() {
		glDepthMask(0);
		auto& shader = loader.GetShader("SSR");
		shader.Activate();
		shader.SetVec3("LightPos", shadowMap.GetLightSpaceCamera().GetPosition());

		glm::mat4 cameraVP = glm::perspective(glm::radians(camera.fov), 
			mainWindow.GetAspect(), near, far) * camera.GetViewMatrix();
		shader.SetMat4("ScreenMat", cameraVP);
		quad.Draw(shader, 
			[&gBuffer](int beginID, const Core::Shader& shader) {
				const char* names[3] = { 
					"DirectIllum", "WorldNormal", "WorldPosAndDepth" 
				};
				for (unsigned int i = 0; i < gBuffer.GetColorBufferNum(); i++)
				{
					Core::Texture::BindTextureOnShader(beginID + i, names[i],
						shader, gBuffer.GetColorBuffer(i));
				}
				return;
			}, nullptr);
		glDepthMask(0xFF);
	});

	SetBasicKeyBindings(mainWindow, camera);
	mainWindow.MainLoop(glm::vec4{0.0});
    return 0;
}