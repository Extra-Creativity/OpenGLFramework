#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <array>
#include <string_view>
#include <span>

namespace OpenGLFramework::Core
{

class Shader
{
public:
    Shader(const std::filesystem::path& vertexShaderFilePath, 
        const std::filesystem::path& fragmentShaderPath);
    Shader(const std::filesystem::path& vertexShaderFilePath, 
        const std::filesystem::path& geometryShaderPath, 
        const std::filesystem::path& fragmentShaderPath);
    Shader(const Shader& another) = delete;
    Shader& operator=(const Shader& another) = delete;
    Shader(Shader&& another) noexcept :shaderID_{another.shaderID_} {
        another.shaderID_ = 0;
    };
    Shader& operator=(Shader&& another) noexcept { 
        if (&another == this) [[unlikely]]
            return *this;
        
        glDeleteProgram(shaderID_);
        shaderID_ = another.shaderID_;
        another.shaderID_ = 0;
    }
    ~Shader() {
        glDeleteProgram(shaderID_);
        return;
    };

    void Activate() const { glUseProgram(shaderID_); };

    void SetBool(const char* name, const bool value) const {
        glUniform1i(glGetUniformLocation(shaderID_, name), (int)value);
    }

    void SetInt(const char* name, const int value) const {
        glUniform1i(glGetUniformLocation(shaderID_, name), value);
    }

    void SetFloat(const char* name, const float value) const {
        glUniform1f(glGetUniformLocation(shaderID_, name), value);
    }

    void SetVec2(const char* name, const glm::vec2 value) const {
        glUniform2fv(glGetUniformLocation(shaderID_, name), 1, &value[0]);
    }
    void SetVec2(const char* name, const float x, const float y) const {
        glUniform2f(glGetUniformLocation(shaderID_, name), x, y);
    }

    void SetVec3(const char* name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(shaderID_, name), 1, &value[0]);
    }
    void SetVec3(const char* name, const float x, const float y, 
        const float z) const 
    {
        glUniform3f(glGetUniformLocation(shaderID_, name), x, y, z);
    }

    void SetVec4(const char* name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(shaderID_, name), 1, &value[0]);
    }
    void SetVec4(const char* name, const float x, const float y,
        const float z, const float w) const 
    {
        glUniform4f(glGetUniformLocation(shaderID_, name), x, y, z, w);
    }

    void SetMat2(const char* name, const glm::mat2& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(shaderID_, name), 1, 
            GL_FALSE, &mat[0][0]);
    }

    void SetMat3(const char* name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(shaderID_, name), 1, 
            GL_FALSE, &mat[0][0]);
    }

    void SetMat4(const char* name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(shaderID_, name), 1, 
            GL_FALSE, &mat[0][0]);
    }

private:
    // Here shaderID actually means OpenGL's program.
    GLuint shaderID_;
    unsigned int CompileShader_(std::string_view shaderContent,
        const GLenum shaderType);
    void LinkShaders_(std::span<unsigned int> shaders);
    void ClearShaders_(std::span<unsigned int> shaders);
};

} // namespace OpenGLFramework::Core
