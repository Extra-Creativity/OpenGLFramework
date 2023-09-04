#include "FrameworkCore/Core_All.h"

#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"
#include "../Base/ImGUIHelper.h"
#include "CornellBox.h"
#include "ShadowMap.h"

struct Test
{
	float near, far;
	ShadowMap& shadowMap;
	CornellBox& box;
};

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
	camera.Translate(camera.Back() * 15.f);

	const auto [width, height] = mainWindow.GetWidthAndHeight();
	ShadowMap shadowMap{ width, height, loader };
	CornellBox box{ loader.GetShader("cornell") };
	
	mainWindow.Register([&] { ShadowMap::Render(shadowMap, box.GetModels()); });

	float near = 1.0f, far = 50.0f;
	mainWindow.Register([&]() { 
		box.Draw(mainWindow.GetAspect(), near, far, camera, shadowMap);
	});

	static Core::Framebuffer buffer{ 800, 600 };
	ExampleBase::ImGuiHelper<Test> helper{ 800, 600, "shadow map view",
		{ near, far, shadowMap, box},
		[](Test& t) { 
			buffer.Clear();
			buffer.UseAsRenderTarget();
			t.box.Draw(t.shadowMap.GetAspect(), t.near, t.far,
				t.shadowMap.GetLightSpaceCamera(), t.shadowMap); 
			ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(
				buffer.GetColorBuffer())), { 800, 600 }, { 0, 1 }, { 1, 0 });
			Core::Framebuffer::RestoreDefaultRenderTarget();
		}
	};
	helper.RegisterOnMainWindow(mainWindow);
	SetBasicKeyBindings(mainWindow, camera);
	mainWindow.MainLoop(glm::vec4{0.0});
    return 0;
}