#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/SkyboxConfig.ini";
	IOExtension::IniFile file{ configPath };

	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };
	
	auto skyBox = file.rootSection("option") == "split" ? 
		Core::SkyBoxTexture{ pathsSection("split_skybox_dir") } :
		Core::SkyBoxTexture{ pathsSection("skybox"), 
			Core::SkyBoxTexture::TextureSegmentType::HorizontalLeft 
		};
	Core::Shader skyBoxShader{
		pathsSection("skybox_vertex_shader"),
		pathsSection("skybox_fragment_shader")
	};
	Core::BasicTriRenderModel cube = Core::Cube::GetBasicTriRenderModel();
	cube.transform.scale = { 10, 10, 10 };
	cube.transform.Translate({ -0.5,-0.5,-0.5 });

	Core::BasicTriRenderModel sucroseModel{ pathsSection("sucrose") };
	Core::Shader sucroseShader{
		pathsSection("sucrose_vertex_shader"),
		pathsSection("sucrose_fragment_shader")
	};
	sucroseModel.transform.scale = { 0.1, 0.1, 0.1 };

	float near = 0.1f, far = 100.0f;

	Core::Camera frontCamera{ { 0, 1, 3.5}, {0, 1, 0}, {0, 0, -1} };
	auto initialViewMatTranslation = frontCamera.GetViewMatrix()[3];
	SetBasicKeyBindings(mainWindow, frontCamera);
	SetBasicButtonBindings(mainWindow, frontCamera);

	auto BindSkybox = [&skyBox](int textureBeginID, Core::Shader& shader)
	{
		glActiveTexture(GL_TEXTURE0 + textureBeginID);
		shader.SetInt("skybox", textureBeginID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyBox.GetID());
	};

	mainWindow.Register([&]() {
		sucroseShader.Activate();
		SetMVP(static_cast<float>(width), static_cast<float>(height), 
			static_cast<float>(near), static_cast<float>(far), 
			sucroseModel, frontCamera, sucroseShader);
		sucroseShader.SetVec3("cameraPos", frontCamera.GetPosition());
		sucroseModel.Draw(sucroseShader, BindSkybox, nullptr);
	});

	mainWindow.Register([&]() {
		glDepthFunc(GL_LEQUAL);
		skyBoxShader.Activate();
		skyBoxShader.SetMat4("model", cube.transform.GetModelMatrix());
		auto viewMat = glm::mat4{ glm::mat3{ frontCamera.GetViewMatrix() } };
		viewMat[3] = initialViewMatTranslation;
		skyBoxShader.SetMat4("view", viewMat);
		SetProjection(width, height, near, far, frontCamera, skyBoxShader);
		cube.Draw(skyBoxShader, BindSkybox, nullptr);
		glDepthFunc(GL_LESS);
	});

	mainWindow.MainLoop({ 0.0, 0.0, 0.0, 0.0 });
	return 0;
}