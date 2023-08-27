#include "BasicSettings.h"

#include "FrameworkCore/Core_All.h"
#include "Utility/IO/IniFile.h"
#include "Utility/IO/IOExtension.h"

#include <cmath>

// This requires IniFile doesn't destruct.
// If not, it's better to return std::vector<std::string> to keep strings valid.
std::vector<const char*> GetAllFilterOptions(
	const IOExtension::IniFile<std::unordered_map>& file)
{
	auto& filterSection = file.rootSection["filters"];
	std::vector<const char*> result;
	for (size_t i = 1;; i++)
	{
		auto entry = filterSection.GetEntry("filter" + std::to_string(i));
		if (!entry.has_value())
			break;
		result.push_back(entry.value().get().c_str());
	}
	return result;
}

int main()
{
	std::filesystem::path configPath = "../../../../../Resources/Configs/FilterConfig.ini";
	IOExtension::IniFile file{ configPath };
	auto filterLists = GetAllFilterOptions(file);

	auto& pathsSection = file.rootSection.GetSubsection("paths")->get();
	auto& windowSection = file.rootSection.GetSubsection("window")->get();
	std::string windowName = windowSection("name");
	unsigned int width = std::stoi(windowSection("width")),
		height = std::stoi(windowSection("height"));

	[[maybe_unused]] auto& contextManager = Core::ContextManager::GetInstance();
	Core::MainWindow mainWindow{ width, height, windowName.c_str() };

	Core::BasicTriRenderModel sucroseModel{ pathsSection("sucrose_resource_dir") };
	Core::BasicTriRenderModel floor{ pathsSection("plane_resource_dir") };
	Core::Shader normalShader{
		pathsSection("vertex_shader_dir"),
		pathsSection("fragment_shader_dir")
	};
	float near = 0.1f, far = 100.0f;

	Core::Camera frontCamera{ {0, 10, 35}, {0, 1, 0}, {0, 0, -1} };
	SetBasicKeyBindings(mainWindow, frontCamera);
	SetBasicButtonBindings(mainWindow, frontCamera);

	const auto depthConfig = Core::TextureParamConfig{
		.wrapS = Core::TextureParamConfig::WrapType::ClampToEdge,
		.wrapT = Core::TextureParamConfig::WrapType::ClampToEdge
	};	// To eliminate top grey shadow in filter.
	Core::Framebuffer buffer{ width, height, depthConfig };

	auto quadOnScreen = Core::Quad::GetBasicTriRenderModel();
	mainWindow.Register(
		[&normalShader, &sucroseModel, &floor, &buffer, &frontCamera, &mainWindow,
		 &depthConfig, near, far]() {
			buffer.Clear();

			normalShader.Activate();
			const auto [width, height] = mainWindow.GetWidthAndHeight();
			if (width != buffer.GetWidth() || height != buffer.GetHeight())
				buffer = Core::Framebuffer{ width, height, depthConfig };

			SetMVP(static_cast<float>(width), static_cast<float>(height),
				near, far, sucroseModel, frontCamera, normalShader);

			normalShader.SetMat4("modelMat", sucroseModel.transform.GetModelMatrix());
			sucroseModel.Draw(normalShader, buffer);

			normalShader.SetMat4("modelMat", floor.transform.GetModelMatrix());
			floor.Draw(normalShader, buffer);
		});

	int option = 0;
	float pixelSigma = sqrt(0.5f), depthSigma = pixelSigma, colorSigma = pixelSigma;
	mainWindow.Register(
		[&filterLists, &option, &pixelSigma, &depthSigma, &colorSigma]() {
			enum Options { NoFilter = 0, GaussianFilter, JointBilateralFilter,
				SobelDetector, Inversion, GrayScale };

			static bool init = true;
			ImGui::Begin("Options");
			if (init)
			{
				ImGui::SetWindowPos({ 50, 50 });
				ImGui::SetWindowSize({ 300, 250 });
				init = false;
			}
			ImGui::Combo("filters", &option, filterLists.data(), 
				static_cast<int>(filterLists.size()));
			if (option != GaussianFilter && option != JointBilateralFilter)
			{
				ImGui::End();
				return;
			}

			ImGui::Text("standard deviations");
			ImGui::SliderFloat("pixel basic", &pixelSigma, 0.1f, 5.0f);
			if (option == JointBilateralFilter)
			{
				ImGui::SliderFloat("depth", &depthSigma, 0.1f, 5.0f);
				ImGui::SliderFloat("direct illumination", &colorSigma, 0.1f, 5.0f);
			}
			ImGui::End();
		});

	Core::Shader basicQuadShader{
		pathsSection("quad_vertex_shader_dir"),
        pathsSection("quad_fragment_shader_dir")
	};
	mainWindow.Register([&quadOnScreen, &buffer, &basicQuadShader, &option,
		&pixelSigma, &depthSigma, &colorSigma]() {
		glDepthMask(0);
		basicQuadShader.Activate();
		basicQuadShader.SetInt("filterOption", option);
		basicQuadShader.SetFloat("xOffset", 1.0f / buffer.GetWidth());
		basicQuadShader.SetFloat("yOffset", 1.0f / buffer.GetHeight());
		basicQuadShader.SetFloat("pixelSigmaSqr", pixelSigma * pixelSigma);
		basicQuadShader.SetFloat("depthSigmaSqr", depthSigma * depthSigma);
		basicQuadShader.SetFloat("colorSigmaSqr", colorSigma * colorSigma);
		quadOnScreen.Draw(basicQuadShader,
			[&buffer](int textureBeginID, const Core::Shader& shader) {
				Core::Texture::BindTextureOnShader(textureBeginID, "colorTexture",
					shader, buffer.GetColorBuffer());
				Core::Texture::BindTextureOnShader(textureBeginID + 1, "depthTexture",
					shader, buffer.GetDepthBuffer());
			}, nullptr);
		glDepthMask(0xFF);
	});
	mainWindow.MainLoop({ 0.0, 0.0, 0.0, 0.0 });
	return 0;
}