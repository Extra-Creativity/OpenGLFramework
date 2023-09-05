#include "FrameworkCore/Core_All.h"

#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"
#include "../Base/ImGUIHelper.h"

#include "CornellBox.h"
#include "ShadowMap.h"
#include "GBuffer.h"

void ResizeBuffers(const Core::MainWindow& mainWindow, ShadowMap& shadowMap,
	GBuffer& gBuffer)
{
	const auto [width, height] = mainWindow.GetWidthAndHeight();
	static auto lastWidth = width, lastHeight = height;
	if (lastWidth != width || lastHeight != height)
	{
		shadowMap.ResizeBuffer(width, height);
		gBuffer.ResizeBuffer(width, height);
		lastWidth = width, lastHeight = height;
	}
}

struct IllumOption { int& option; };
void SetIllumOption(IllumOption& opt) {
	const char* names[] = { "Direct", "Specular", "Diffuse"};
	ImGui::Combo("Options", &opt.option, names,
		static_cast<int>(std::size(names)));
	return;
}

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
	mainWindow.Register([&] { ShadowMap::Render(shadowMap, box.GetModels()); });

	GBuffer gBuffer{ width, height };
	float near = 1.0f, far = 50.0f;
	mainWindow.Register([&]() {
		const auto& frameBuffer = gBuffer.GetFrameBuffer();
		frameBuffer.Clear();
		frameBuffer.UseAsRenderTarget();
		box.Draw(mainWindow.GetAspect(), near, far, camera, shadowMap);
		frameBuffer.RestoreDefaultRenderTarget();
	});

	mainWindow.Register(std::bind_front(ResizeBuffers, std::cref(mainWindow), 
		std::ref(shadowMap), std::ref(gBuffer)));

	int option = 0;
	ExampleBase::ImGuiHelper<IllumOption> illumOption{ 200, 100, "Illumination Option", 
		{option}, SetIllumOption
	};
	illumOption.RegisterOnMainWindow(mainWindow);

	auto quad = Core::Quad::GetBasicTriRenderModel();
	mainWindow.Register([&]() {
		glDepthMask(0);
		auto& shader = loader.GetShader("SSR");
		shader.Activate();
		shader.SetInt("Option", option);
		shader.SetVec3("LightPos", shadowMap.GetLightSpaceCamera().GetPosition());

		glm::mat4 cameraVP = glm::perspective(glm::radians(camera.fov), 
			mainWindow.GetAspect(), near, far) * camera.GetViewMatrix();
		shader.SetMat4("ScreenMat", cameraVP);
		quad.Draw(shader, [&gBuffer](int beginID, const Core::Shader& shader) {
			gBuffer.BindAsTextures(beginID, shader);
		}, nullptr);

		glDepthMask(0xFF);
	});

	SetBasicKeyBindings(mainWindow, camera);
	mainWindow.MainLoop(glm::vec4{0.0});
    return 0;
}