#include "Shader.h"
#include "IOExtension.h"
#include <iostream>

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, const std::filesystem::path& fragmentShaderPath)
{
    constexpr int shaderNum = 2;
    m_shaders.reserve(shaderNum);

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShader_();
    return;
};

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, const std::filesystem::path& geometryShaderPath, const std::filesystem::path& fragmentShaderPath)
{
    constexpr int shaderNum = 3;
    m_shaders.reserve(shaderNum);

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(geometryShaderPath);
    CompileShader_(fileContent, GL_GEOMETRY_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShader_();
    return;
};

void Shader::CompileShader_(const std::string& shaderContent, GLenum shaderType)
{
    GLuint newShader = glCreateShader(shaderType);
    const char* contentPtr = shaderContent.c_str();
    glShaderSource(newShader, 1, &contentPtr, NULL);
    glCompileShader(newShader);

    GLint compileSuccess = 0;
    glGetShaderiv(newShader, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_TRUE) [[likely]]
    {
        m_shaders.push_back(newShader);
        return;
    }
    // Else compile fails.
    GLint length = 0;
    glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &length);
    std::vector<GLchar> errorLog(length);
    GLchar* logPtr = errorLog.data();
    glGetShaderInfoLog(newShader, length - 1, &length, logPtr);
    IOExtension::LogError(std::source_location::current(), std::string_view{logPtr});

    glDeleteShader(newShader);
    return;
};

void Shader::ClearShader_()
{
    for (auto& shader : m_shaders)
    {
        glDeleteShader(shader);
    }
    m_shaders.resize(0);
    return;
}

void Shader::LinkShader_()
{
    GLuint newShaderAssembly = glCreateProgram();
    for (auto& shader : m_shaders)
    {
        glAttachShader(newShaderAssembly, shader);
    }
    glLinkProgram(newShaderAssembly);

    GLint linkSuccess = 0;
    glGetProgramiv(newShaderAssembly, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_TRUE)[[likely]]
    {
        shaderID = newShaderAssembly;
        ClearShader_();
        return;
    }
    // Else link fails.
    GLint length = 0;
    glGetProgramiv(newShaderAssembly, GL_INFO_LOG_LENGTH, &length);
    std::vector<GLchar> errorLog(length);
    GLchar* logPtr = errorLog.data();
    glGetProgramInfoLog(newShaderAssembly, length - 1, &length, logPtr);
    IOExtension::LogError(std::source_location::current(), std::string_view{ logPtr });

    glDeleteProgram(newShaderAssembly);
    ClearShader_();
    return;
}

Shader::~Shader()
{
    glDeleteProgram(shaderID);
    return;
}

