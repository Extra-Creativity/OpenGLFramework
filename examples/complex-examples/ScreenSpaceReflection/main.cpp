#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"
#include "CornellBox.h"
#include "FrameworkCore/Core_All.h"

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
	
	float near = 1.0f, far = 50.0f;
	SetBasicKeyBindings(mainWindow, camera);

	CornellBox box{ loader.GetShader("cornell") };
	mainWindow.Register([&]() { box.Draw(mainWindow.GetAspect(), near, far, camera); });
	mainWindow.MainLoop(glm::vec4{0.0});
    return 0;
}