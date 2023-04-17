#include "../Base/BasicSettings.h"
#include "../Base/AssetLoader.h"
#include "../Base/ImGUIHelper.h"

#include "FrameworkCore/Core_All.h"

#include "ShadowMap.h"
#include "ScreenShader.h"

#include <array>

struct ShadowOptionData
{
	int option = 1;
	std::array<const char*, 3> optionList{
		"Hard Shadow(No bias)",
		"Hard Shadow(Add bias)",
		"PCF Shadow",
	};
};

void SetShadowOption(ShadowOptionData& data)
{
	ImGui::Combo("shadow options", &data.option, data.optionList.data(),
		static_cast<int>(data.optionList.size()));
	return;
}

struct LightPosData
{
	Core::Camera& lightSpaceCamera;
	glm::vec3 rotationCenter{ 0, 10, 0 };
	glm::vec3 rotationAngles{ -45.0f, -25.0f, 0.0f };
};

void SetLightPos(LightPosData& data)
{
	ImGui::SliderFloat3("rotation", &data.rotationAngles.x, -180, 180);
	for (int i = 0; i < 3; i++)
	{
		glm::vec3 axis{ 0.0f };
		axis[i] = 1.0f;
		data.lightSpaceCamera.RotateAroundCenter(data.rotationAngles[i], 
			axis, data.rotationCenter);
	}
}

void RestoreLightPos(LightPosData& data)
{
	for (int i = 2; i >= 0; i--)
	{
		glm::vec3 axis{ 0.0f };
		axis[i] = 1.0f;
		data.lightSpaceCamera.RotateAroundCenter(-data.rotationAngles[i],
			axis, data.rotationCenter);
	}
}

struct BasicInfoData
{
	Core::MainWindow& mainWindow;
	int frameCnt{};
	float lastFPS{};
	static const int statInterval = 100;
};

void StatBasicInfo(BasicInfoData& data)
{
	if (data.frameCnt % BasicInfoData::statInterval == 0) {
		data.lastFPS = 1 / data.mainWindow.deltaTime;
	}
	data.frameCnt++;
	ImGui::Text("FPS: %f", data.lastFPS);
}

void ResizeBufferToScreen(Core::MainWindow& mainWindow,
	Core::Framebuffer& buffer)
{
	const auto [width, height] = mainWindow.GetWidthAndHeight();
	if (width == buffer.GetWidth() && height == buffer.GetHeight())
		return;
	buffer.Resize(width, height);
	return;
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

	const auto [width, height] = mainWindow.GetWidthAndHeight();

	ExampleBase::ImGuiHelper<ShadowOptionData> shadowOptionSetter{
		300, 150, "Options", {}, SetShadowOption };
	ExampleBase::RegisterHelperOnMainWindow(mainWindow, shadowOptionSetter);

	ExampleBase::ImGuiHelper<BasicInfoData> basicInfoShow{
		150, 50, "Basic Info", {mainWindow}, StatBasicInfo
	};
	ExampleBase::RegisterHelperOnMainWindow(mainWindow, basicInfoShow);

	ShadowMap shadowMap{ width, height, loader };
	mainWindow.Register(std::bind(ResizeBufferToScreen,
		std::ref(mainWindow), std::ref(shadowMap.buffer))
	);
	mainWindow.Register(std::bind(ShadowMap::Render,
		std::ref(shadowMap), std::ref(loader.GetModelContainer()))
	);

	ScreenShader screen{ loader };
	mainWindow.Register(std::bind(ScreenShader::Render,
		std::ref(screen), std::ref(shadowMap),
		std::ref(shadowOptionSetter.GetData().option), 
		std::ref(loader.GetModelContainer()))
	);

	ExampleBase::ImGuiHelper<LightPosData> lightSetter{
		250, 100, "Light Adjustment", {shadowMap.GetLightSpaceCamera()},
		SetLightPos, RestoreLightPos
	};
	ExampleBase::RegisterHelperOnMainWindow(mainWindow, lightSetter);

	SetBasicKeyBindings(mainWindow, screen.GetCamera());
	SetBasicButtonBindings(mainWindow, screen.GetCamera());

	mainWindow.MainLoop({ 1.0, 1.0, 1.0, 0.0 });
	return 0;
}