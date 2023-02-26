#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"
#include "Utility/IO/IOExtension.h"

#include <iostream>

using namespace OpenGLFramework;

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/BlinnPhongConfig.ini";
	IOExtension::IniFile file{ configPath };

	auto& modelsSection = file.rootSection.GetSubsection("models")->get();
	auto& nameSection = file.rootSection.GetSubsection("name")->get();
	std::string windowName = nameSection("window_name");

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ 800, 600, windowName.c_str() };

	std::string modelDir = "resource_dir", vertexShaderDir = "vertex_shader_dir",
		fragmentShaderDir = "fragment_shader_dir";

	auto& sucroseSection = modelsSection.GetSubsection("sucrose")->get();
	auto& floorSection = modelsSection.GetSubsection("floor")->get();

	Core::BasicTriRenderModel sucroseModel{ sucroseSection(modelDir)};
	Core::BasicTriRenderModel floor{ floorSection(modelDir) };
	Core::Shader sucroseShader{
		sucroseSection(vertexShaderDir),
		sucroseSection(fragmentShaderDir)
	};
	Core::Shader floorShader{
		floorSection(vertexShaderDir),
		floorSection(fragmentShaderDir)
	};
	Core::Camera frontCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} };

    SetBasicKeyBindings(mainWindow, frontCamera);
    SetBasicButtonBindings(mainWindow, frontCamera);
	SetBasicTransformSubwindow(mainWindow, sucroseModel);
	float near = 0.1f, far = 100.0f;
	glm::vec3 lightPosition = { -3, 12, 7.5 };
	glm::vec3 lightColor = { 1, 1, 1 };

	mainWindow.Register([&sucroseModel, &lightPosition, &lightColor]() {
		static bool init = true;
		ImGui::Begin("Light Adjustment");
		if (init)
		{
			ImGui::SetWindowPos({ 50, 150 });
			ImGui::SetWindowSize({ 250, 100 });
			init = false;
		}
		ImGui::SliderFloat3("position", &lightPosition.x, -20, 20);
		ImGui::SliderFloat3("color", &lightColor.r, 0, 1);
		ImGui::End();
		}
	);

	mainWindow.Register(
		[&sucroseShader, &sucroseModel, &frontCamera, &mainWindow, &lightPosition, &lightColor, near, far]() {
			glEnable(GL_DEPTH_TEST);
			sucroseShader.Activate();
			const auto [width, height] = mainWindow.GetWidthAndHeight();
			SetMVP(static_cast<float>(width), static_cast<float>(height),
				near, far, sucroseModel, frontCamera, sucroseShader);
			sucroseShader.SetVec3("lightPos", lightPosition);
			sucroseShader.SetVec3("lightColor", lightColor);
			sucroseShader.SetVec3("cameraPos", frontCamera.GetPosition());
			sucroseModel.Draw(sucroseShader);
		});

	mainWindow.Register(
		[&floorShader, &floor, &frontCamera, &mainWindow, near, far]() {
			floorShader.Activate();
			const auto [width, height] = mainWindow.GetWidthAndHeight();
			SetMVP(static_cast<float>(width), static_cast<float>(height),
				near, far, floor, frontCamera, floorShader);
			floor.Draw(floorShader);
		}
	);
	mainWindow.MainLoop({0, 0, 0, 1});
    return 0;
}