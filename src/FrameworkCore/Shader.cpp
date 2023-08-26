#include "Shader.h"
#include "Utility/IO/IOExtension.h"
#include <vector>

namespace OpenGLFramework::Core
{

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, 
    const std::filesystem::path& fragmentShaderPath)
{
    std::array<unsigned int, 2> shaders;

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    shaders[0] = CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    shaders[1] = CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShaders_(shaders);
    return;
};

Shader::Shader(const std::filesystem::path& vertexShaderFilePath, 
    const std::filesystem::path& geometryShaderPath, 
    const std::filesystem::path& fragmentShaderPath)
{
    std::array<unsigned int, 3> shaders;

    std::string fileContent = IOExtension::ReadAll(vertexShaderFilePath);
    shaders[0] = CompileShader_(fileContent, GL_VERTEX_SHADER);

    fileContent = IOExtension::ReadAll(geometryShaderPath);
    shaders[1] = CompileShader_(fileContent, GL_GEOMETRY_SHADER);

    fileContent = IOExtension::ReadAll(fragmentShaderPath);
    shaders[2] = CompileShader_(fileContent, GL_FRAGMENT_SHADER);

    LinkShaders_(shaders);
    return;
};

unsigned int Shader::CompileShader_(std::string_view shaderContent, 
    const GLenum shaderType)
{
    GLuint newShader = glCreateShader(shaderType);
    const char* contentPtr = shaderContent.data();
    glShaderSource(newShader, 1, &contentPtr, NULL);
    glCompileShader(newShader);

    GLint compileSuccess = 0;
    glGetShaderiv(newShader, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_TRUE) [[likely]]
        return newShader;
    // Else compile fails.
    GLint length = 0;
    glGetShaderiv(newShader, GL_INFO_LOG_LENGTH, &length);
    std::vector<GLchar> errorLog(length);
    GLchar* logPtr = errorLog.data();
    glGetShaderInfoLog(newShader, length - 1, &length, logPtr);
    IOExtension::LogError(std::string_view{logPtr});

    glDeleteShader(newShader);
    return 0;
};

void Shader::ClearShaders_(std::span<unsigned int> shaders)
{
    for (auto& shader : shaders)
        glDeleteShader(shader);
    return;
}

void Shader::LinkShaders_(std::span<unsigned int> shaders)
{
    GLuint newShaderAssembly = glCreateProgram();
    for (auto& shader : shaders)
    {
        glAttachShader(newShaderAssembly, shader);
    }
    glLinkProgram(newShaderAssembly);

    GLint linkSuccess = 0;
    glGetProgramiv(newShaderAssembly, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_TRUE)[[likely]]
    {
        shaderID_ = newShaderAssembly;
        ClearShaders_(shaders);
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
    ClearShaders_(shaders);
    return;
}

} // namespace OpenGLFramework::Core