#include "Shader.h"
#include "Utility/IO/IOExtension.h"

#include <iostream>

namespace OpenGLFramework::Core
{

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, 
    const std::filesystem::path& fragmentShaderPath)
{
    constexpr int shaderNum = 2;
    shaders_.reserve(shaderNum);

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShader_();
    return;
};

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, 
    const std::filesystem::path& geometryShaderPath, 
    const std::filesystem::path& fragmentShaderPath)
{
    constexpr int shaderNum = 3;
    shaders_.reserve(shaderNum);

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(geometryShaderPath);
    CompileShader_(fileContent, GL_GEOMETRY_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShader_();
    return;
};

void Shader::CompileShader_(const std::string& shaderContent, const GLenum shaderType)
{
    GLuint newShader = glCreateShader(shaderType);
    const char* contentPtr = shaderContent.c_str();
    glShaderSource(newShader, 1, &contentPtr, NULL);
    glCompileShader(newShader);

    GLint compileSuccess = 0;
    glGetShaderiv(newShader, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_TRUE) [[likely]]
    {
        shaders_.push_back(newShader);
        return;
    }
    // Else compile fails.
    GLint length = 0;
    glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &length);
    std::vector<GLchar> errorLog(length);
    GLchar* logPtr = errorLog.data();
    glGetShaderInfoLog(newShader, length - 1, &length, logPtr);
    IOExtension::LogError(std::string_view{logPtr});

    glDeleteShader(newShader);
    return;
};

void Shader::ClearShader_()
{
    for (auto& shader : shaders_)
    {
        glDeleteShader(shader);
    }
    shaders_.resize(0);
    return;
}

void Shader::LinkShader_()
{
    GLuint newShaderAssembly = glCreateProgram();
    for (auto& shader : shaders_)
    {
        glAttachShader(newShaderAssembly, shader);
    }
    glLinkProgram(newShaderAssembly);

    GLint linkSuccess = 0;
    glGetProgramiv(newShaderAssembly, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_TRUE)[[likely]]
    {
        shaderID_ = newShaderAssembly;
        ClearShader_();
        return;
    }
    // Else link fails.
    GLint length = 0;
    glGetProgramiv(newShaderAssembly, GL_INFO_LOG_LENGTH, &length);
    std::vector<GLchar> errorLog(length);
    GLchar* logPtr = errorLog.data();
    glGetProgramInfoLog(newShaderAssembly, length - 1, &length, logPtr);
    IOExtension::LogError(std::string_view{ logPtr });

    glDeleteProgram(newShaderAssembly);
    ClearShader_();
    return;
}

Shader::~Shader()
{
    glDeleteProgram(shaderID_);
    return;
}

} // namespace OpenGLFramework::Core