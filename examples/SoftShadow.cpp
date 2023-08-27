#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/Generator/Generator.h"
#include "Utility/IO/IniFile.h"
#include "Utility/IO/IOExtension.h"

#include <ranges>
#include <array>

using OpenGLFramework::Coroutine::Generator;

static std::unordered_map<std::string, Core::BasicTriRenderModel> models;
static std::unordered_map<std::string, Core::Shader> shaders;

void InitializeModelPools(IOExtension::IniFile<std::unordered_map>& file) 
{
	auto& modelPaths = file.rootSection.GetSubsection("paths.models")->get();
	for (const auto& [name, path] : modelPaths.GetRawEntries())
	{
		models.emplace(name, path);
	}
	return;
};

void InitializeShaderPools(IOExtension::IniFile<std::unordered_map>& file)
{
	auto& shaderPaths = file.rootSection.GetSubsection("paths.shaders")->get();
	for (const auto& [_, section] : shaderPaths.GetRawSubsections())
	{
		auto geometryShader = section.GetEntry("geometry_shader");
		if (!geometryShader.has_value())
		{
			shaders.emplace(section.GetEntry("name")->get(), Core::Shader{
				section.GetEntry("vertex_shader")->get(),
				section.GetEntry("fragment_shader")->get()
			});
		}
		else {
			shaders.emplace(section.GetEntry("name")->get(), Core::Shader{
				section.GetEntry("vertex_shader")->get(),
				geometryShader->get(),
				section.GetEntry("fragment_shader")->get()
			});
		}
	}
	return;
};

void InitializeAssets(IOExtension::IniFile<std::unordered_map>& file)
{
	InitializeModelPools(file);
	InitializeShaderPools(file);
	return;
}

Generator<int> SetLightPosition(Core::Camera& lightSpaceCamera)
{
	glm::vec3 rotationCenter = { 0, 10, 0 };
	// A tuned angle to get a good shadow.
	glm::vec3 rotationAngles{ -45.0f, -25.0f, 0.0f };
	co_yield 0;

	ImGui::Begin("Light Adjustment");
	ImGui::SetWindowPos({ 50, 50 });
	ImGui::SetWindowSize({ 250, 100 });
	while (true)
	{
		ImGui::SliderFloat3("rotation", &rotationAngles.x, -180, 180);
		ImGui::End();
		for (int i = 0; i < 3; i++)
		{
			glm::vec3 axis{ 0.0f };
			axis[i] = 1.0f;
			lightSpaceCamera.RotateAroundCenter(rotationAngles[i], axis,
				rotationCenter);
		}
		co_yield 0;

		// restore the camera.
		for (int i = 2; i >= 0; i--)
		{
			glm::vec3 axis{ 0.0f };
			axis[i] = 1.0f;
			lightSpaceCamera.RotateAroundCenter(-rotationAngles[i], axis,
				rotationCenter);
		}
		ImGui::Begin("Light Adjustment");
	}
	co_return;
}

Generator<int> SetShadowOption(int& option)
{
	std::array<const char*, 3> optionList{
		"Hard Shadow(No bias)",
		"Hard Shadow(Add bias)",
		"PCF Shadow",
	};
	co_yield 0;

	ImGui::Begin("Options");
	ImGui::SetWindowPos({ 50, 200 });
	ImGui::SetWindowSize({ 300, 150 });

	while (true)
	{
		ImGui::Combo("filters", &option, optionList.data(),
			static_cast<int>(optionList.size()));
		ImGui::End();
		co_yield 0;

		ImGui::Begin("Options");
	}
	co_return;
}

Generator<int> ShowBasicInfo(Core::MainWindow& mainWindow)
{
	int frameCnt = 0;
	float lastFPS = 0;
	co_yield 0;

	ImGui::Begin("Basic Info");
	ImGui::SetWindowPos({ 50, 500 });
	ImGui::SetWindowSize({ 150, 50 });

	while (true)
	{
		if (frameCnt % 100 == 0)
		{
			lastFPS = 1 / mainWindow.GetDeltaTime();
		}
		ImGui::Text("FPS: %f", lastFPS);
		ImGui::End();
		frameCnt++;
		co_yield 0;
		ImGui::Begin("Basic Info");
	}
}

void RenderShadowMap(Core::Framebuffer& buffer, Core::Camera& lightSpaceCamera)
{
	using enum Core::Framebuffer::BasicClearMode;
	buffer.Clear({ DepthClear }, false);

	auto& shadowMapShader = shaders.find("shadow map")->second;
	float near = 0.1f, far = 100.0f;

	glm::mat4 projection = glm::perspective(glm::radians(lightSpaceCamera.fov),
		buffer.GetAspect(), near, far);
	glm::mat4 lightSpaceMat = projection * lightSpaceCamera.GetViewMatrix();

	shadowMapShader.Activate();
	shadowMapShader.SetMat4("lightSpaceMat", lightSpaceMat);

	for (auto& [name, model] : models)
	{
		shadowMapShader.SetMat4("modelMat", model.transform.GetModelMatrix());
		model.Draw(shadowMapShader, buffer);
	}

	// Set light camera matrix for screen shader.
	auto& screenShader = shaders.find("screen")->second;
	screenShader.Activate();
	screenShader.SetMat4("lightSpaceMat", lightSpaceMat);
	return;
}

Core::TextureParamConfig depthConfig = {
	.minFilter = Core::TextureParamConfig::MinFilterType::Linear,
	.wrapS = Core::TextureParamConfig::WrapType::ClampToBorder,
	.wrapT = Core::TextureParamConfig::WrapType::ClampToBorder,
	.wrapR = Core::TextureParamConfig::WrapType::ClampToBorder,
	.auxHandle = [] {
		float border[] = { 1.0,1.0,1.0,1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	}
};

void ResizeBufferToScreen(Core::MainWindow& mainWindow,
	Core::Framebuffer& buffer)
{
	const auto [width, height] = mainWindow.GetWidthAndHeight();
	if (width == buffer.GetWidth() && height == buffer.GetHeight())
		return;
	buffer = Core::Framebuffer{ width, height, depthConfig, {} };
	return;
};

void RenderScreen(Core::Framebuffer& shadowMapBuffer, Core::Camera& camera,
	Core::Camera& lightSpaceCamera, const int& shadowOption)
{
	auto& screenShader = shaders.find("screen")->second;

	float near = 0.1f, far = 100.0f;
	screenShader.Activate();
	screenShader.SetVec3("lightPos", lightSpaceCamera.GetPosition());
	screenShader.SetVec3("viewPos", camera.GetPosition());
	screenShader.SetMat4("view", camera.GetViewMatrix());
	screenShader.SetMat4("projection", glm::perspective(camera.fov,
		shadowMapBuffer.GetAspect(), near, far));
	screenShader.SetInt("shadowOption", shadowOption);

	using enum Core::Framebuffer::BasicClearMode;
	auto UseShadowMap = 
		[&shadowMapBuffer](int textureBeginID, const Core::Shader& shader) {
			Core::Texture::BindTextureOnShader(textureBeginID, "shadowMap",
				shader, shadowMapBuffer.GetDepthBuffer());
		};

	for (auto& [name, model] : models)
	{
		screenShader.SetMat4("model", model.transform.GetModelMatrix());
		model.Draw(screenShader, UseShadowMap, nullptr);
	}
	return;
}

Core::MainWindow LoadMainWindow(IOExtension::IniFile<std::unordered_map>& file)
{
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));
	return { width, height, windowName.c_str() };
}

void Increment(auto& it) { it++; }

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/SoftShadowConfig.ini";
	IOExtension::IniFile file{ configPath };
	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	auto mainWindow = LoadMainWindow(file);
	InitializeAssets(file);

	const auto [width, height] = mainWindow.GetWidthAndHeight();
	Core::Framebuffer buffer{ width, height, depthConfig, {} };

	using IterType = Generator<int>::Iter;

	mainWindow.Register(std::bind(ResizeBufferToScreen, 
		std::ref(mainWindow), std::ref(buffer))
	);

	Core::Camera lightSpaceCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} },
		normalCamera{ {-30, 10, 18}, {0, 1, 0}, {30, 0, -18} };
	SetBasicKeyBindings(mainWindow, normalCamera);
	SetBasicButtonBindings(mainWindow, normalCamera);

	auto lightSetter = SetLightPosition(lightSpaceCamera);
	mainWindow.Register(std::bind(
		Increment<IterType>, lightSetter.begin())
	);
	int option = 1;
	auto shadowOptionSetter = SetShadowOption(option);
	mainWindow.Register(std::bind(
		Increment<IterType>, shadowOptionSetter.begin())
	);
	auto basicInfoShow = ShowBasicInfo(mainWindow);
	mainWindow.Register(std::bind(
		Increment<IterType>, basicInfoShow.begin())
	);

	mainWindow.Register(std::bind(RenderShadowMap, 
		std::ref(buffer), std::ref(lightSpaceCamera))
	);
	mainWindow.Register(std::bind(RenderScreen, 
		std::ref(buffer), std::ref(normalCamera), 
		std::ref(lightSpaceCamera), std::ref(option))
	);
	mainWindow.MainLoop({ 1.0, 1.0, 1.0, 0.0 });
	return 0;
}