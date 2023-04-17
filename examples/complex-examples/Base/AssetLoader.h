#pragma once
#include "Utility/IO/IniFile.h"
#include "FrameworkCore/Model.h"
#include "FrameworkCore/Shader.h"
#include "FrameworkCore/MainWindow.h"

namespace ExampleBase
{
class AssetLoader
{
public:
    using ModelContainer = std::unordered_map<std::string,
        OpenGLFramework::Core::BasicTriRenderModel>;
    using ShaderContainer = std::unordered_map<std::string,
        OpenGLFramework::Core::Shader>;

    AssetLoader(const std::filesystem::path& path, 
        const std::filesystem::path&, const std::filesystem::path&);
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = delete;
    AssetLoader& operator=(AssetLoader&&) = delete;

    OpenGLFramework::Core::BasicTriRenderModel& GetModel(const std::string&);
    OpenGLFramework::Core::Shader& GetShader(const std::string&);
    auto& GetModelContainer() { return models_; }
    auto& GetShaderContainer() { return shaders_; }
    OpenGLFramework::Core::MainWindow& GetMainWindowInstance();

private:
    void LoadModels_(
        OpenGLFramework::IOExtension::IniFile<std::unordered_map>&,
        const std::filesystem::path&);
    void LoadShaders_(
        OpenGLFramework::IOExtension::IniFile<std::unordered_map>&,
        const std::filesystem::path&);
    void LoadMainWindow_(
        OpenGLFramework::IOExtension::IniFile<std::unordered_map>&);

    OpenGLFramework::Core::MainWindow mainWindow_;
    std::string windowName_;
    ModelContainer models_;
    ShaderContainer shaders_;
};
}