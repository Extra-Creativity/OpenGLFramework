#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/Generator/Generator.h"
#include "Utility/IO/IniFile.h"
#include "Utility/IO/IOExtension.h"

#include <ranges>

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
	ImGui::SetWindowPos({ 50, 150 });
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

void RenderShadowMap(Core::MainWindow& mainWindow,
	Core::Framebuffer& buffer, Core::Camera& lightSpaceCamera)
{
	auto& shadowMapShader = shaders.find("shadow map")->second;
	float near = 0.1f, far = 100.0f;

	using enum Core::Framebuffer::BasicClearMode;
	glm::mat4 projection = glm::perspective(glm::radians(lightSpaceCamera.fov),
		buffer.GetAspect(), near, far);
	glm::mat4 lightSpaceMat = projection * lightSpaceCamera.GetViewMatrix();

	shadowMapShader.Activate();
	shadowMapShader.SetMat4("lightSpaceMat", lightSpaceMat);

	// The first drawn with clear depth.
	auto& firstModel = models.begin()->second;
	shadowMapShader.SetMat4("modelMat", firstModel.transform.GetModelMatrix());
	buffer.SetClearMode({ DepthClear });
	firstModel.Draw(shadowMapShader, buffer);
	
	for (auto& [name, model] : models | std::views::drop(1))
	{
		shadowMapShader.SetMat4("modelMat", model.transform.GetModelMatrix());
		buffer.SetClearMode({ None });
		model.Draw(shadowMapShader, buffer);
	}

	// Set light camera matrix for screen shader.
	auto& screenShader = shaders.find("screen")->second;
	screenShader.Activate();
	screenShader.SetMat4("lightSpaceMat", lightSpaceMat);
	return;
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

void RenderScreen(Core::MainWindow& mainWindow, 
	Core::Framebuffer& shadowMapBuffer, Core::Camera& camera,
	Core::Camera& lightSpaceCamera)
{
	auto& screenShader = shaders.find("screen")->second;

	float near = 0.1f, far = 100.0f;
	screenShader.Activate();
	screenShader.SetVec3("lightPos", lightSpaceCamera.GetPosition());
	screenShader.SetVec3("viewPos", camera.GetPosition());
	screenShader.SetMat4("view", camera.GetViewMatrix());
	screenShader.SetMat4("projection", glm::perspective(camera.fov,
		shadowMapBuffer.GetAspect(), near, far));

	using enum Core::Framebuffer::BasicClearMode;
	auto UseShadowMap = 
		[&shadowMapBuffer](int textureBeginID, Core::Shader& shader) {
			glActiveTexture(GL_TEXTURE0 + textureBeginID);
			shader.SetInt("shadowMap", textureBeginID);
			glBindTexture(GL_TEXTURE_2D, shadowMapBuffer.GetDepthBuffer());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
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

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/SoftShadowConfig.ini";
	IOExtension::IniFile file{ configPath };
	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	auto mainWindow = LoadMainWindow(file);
	InitializeAssets(file);

	const auto [width, height] = mainWindow.GetWidthAndHeight();
	Core::Framebuffer buffer{ width, height,
		Core::Framebuffer::BasicBufferType::OnlyReadableDepthBuffer };

	using IterType = Generator<int>::Iter;

	mainWindow.Register(std::bind(ResizeBufferToScreen, 
		std::ref(mainWindow), std::ref(buffer))
	);

	Core::Camera lightSpaceCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} },
		normalCamera{ {-30, 10, 18}, {0, 1, 0}, {30, 0, -18} };
	SetBasicKeyBindings(mainWindow, normalCamera);
	SetBasicButtonBindings(mainWindow, normalCamera);

	auto lightSetter = SetLightPosition(lightSpaceCamera);
	mainWindow.Register(std::bind(std::advance<IterType, int>, 
		lightSetter.begin(), 1)
	);
	mainWindow.Register(std::bind(RenderShadowMap, 
		std::ref(mainWindow), std::ref(buffer), std::ref(lightSpaceCamera))
	);
	mainWindow.Register(std::bind(RenderScreen, 
		std::ref(mainWindow), std::ref(buffer), std::ref(normalCamera), 
		std::ref(lightSpaceCamera))
	);
	mainWindow.MainLoop({ 1.0, 1.0, 1.0, 0.0 });
	return 0;
}