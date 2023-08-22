#pragma once
#include "Utility/IO/IniFile.h"
#include "FrameworkCore/Model.h"
#include "FrameworkCore/Shader.h"
#include "FrameworkCore/MainWindow.h"

#include <string_view>

namespace ExampleBase
{
class AssetLoader
{
    struct StringHasher
    {
        using is_transparent = void;
        auto operator()(std::string_view s) const { 
            return std::hash<std::string_view>{}(s);
        }
    };
public:
    using ModelContainer = std::unordered_map<std::string,
        OpenGLFramework::Core::BasicTriRenderModel, StringHasher, std::equal_to<>>;
    using ShaderContainer = std::unordered_map<std::string,
        OpenGLFramework::Core::Shader, StringHasher, std::equal_to<>>;

    AssetLoader(const std::filesystem::path& path, 
        const std::filesystem::path&, const std::filesystem::path&);
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader& operator=(const AssetLoader&) = delete;
    AssetLoader(AssetLoader&&) = delete;
    AssetLoader& operator=(AssetLoader&&) = delete;

    OpenGLFramework::Core::BasicTriRenderModel& GetModel(std::string_view);
    OpenGLFramework::Core::Shader& GetShader(std::string_view);
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