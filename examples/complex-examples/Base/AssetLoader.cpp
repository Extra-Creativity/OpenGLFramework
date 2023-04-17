#include "AssetLoader.h"
#include "Utility/IO/IOExtension.h"

using namespace OpenGLFramework;

namespace ExampleBase
{

AssetLoader::AssetLoader(const std::filesystem::path& path,
	const std::filesystem::path& modelRootPath, 
	const std::filesystem::path& shaderRootPath)
{
    IOExtension::IniFile<std::unordered_map> file{ path };
	LoadMainWindow_(file);
	LoadModels_(file, modelRootPath);
    LoadShaders_(file, shaderRootPath);
    return;
}

Core::BasicTriRenderModel& AssetLoader::GetModel(const std::string& name)
{
	auto it = models_.find(name);
	if (it == models_.end()) [[unlikely]]
	{
		IOExtension::LogError("No key called " + name + 
			" in model pool of asset loader");
		throw std::runtime_error{"AssetLoader key error."};
	};
	return it->second;
}

Core::Shader& AssetLoader::GetShader(const std::string& name)
{
	auto it = shaders_.find(name);
	if (it == shaders_.end())[[unlikely]]
	{
		IOExtension::LogError("No key called " + name +
			" in shader pool of asset loader");
		throw std::runtime_error{ "AssetLoader key error." };
	};
	return it->second;
}

Core::MainWindow& AssetLoader::GetMainWindowInstance()
{
	return mainWindow_;
}

void AssetLoader::LoadModels_(IOExtension::IniFile<std::unordered_map>& file,
	const std::filesystem::path& modelRootPath)
{
    auto& modelPaths = file.rootSection.GetSubsection("paths.models")->get();
    for (const auto& [name, path] : modelPaths.GetRawEntries())
    {
        models_.emplace(name, modelRootPath / path);
    }
    return;
}

void AssetLoader::LoadShaders_(IOExtension::IniFile<std::unordered_map>& file,
	const std::filesystem::path& shaderRootPath)
{
	auto& shaderPaths = file.rootSection.GetSubsection("paths.shaders")->get();
	for (const auto& [_, section] : shaderPaths.GetRawSubsections())
	{
		auto geometryShader = section.GetEntry("geometry_shader");
		if (!geometryShader.has_value())
		{
			shaders_.emplace(section.GetEntry("name")->get(), Core::Shader{
				shaderRootPath / section.GetEntry("vertex_shader")->get(),
				shaderRootPath / section.GetEntry("fragment_shader")->get()
			});
		}
		else {
			shaders_.emplace(section.GetEntry("name")->get(), Core::Shader{
				shaderRootPath / section.GetEntry("vertex_shader")->get(),
				shaderRootPath / geometryShader->get(),
				shaderRootPath / section.GetEntry("fragment_shader")->get()
			});
		}
	}
	return;
}

void AssetLoader::LoadMainWindow_(
    IOExtension::IniFile<std::unordered_map>& file)
{
	auto windowSection = file.rootSection.GetSubsection("window");
	const auto& windowParams = windowSection->get().GetRawEntries();
	windowName_ = windowParams.find("name")->second;
	mainWindow_ = {
		static_cast<unsigned int>(std::stoi(windowParams.find("width")->second)),
			static_cast<unsigned int>(std::stoi(windowParams.find("height")->second)),
			windowName_.c_str()
	};
    return;
}

} // namespace ExampleBase