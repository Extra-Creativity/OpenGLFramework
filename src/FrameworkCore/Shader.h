#pragma once
#ifndef OPENGLFRAMEWORK_CORE_SHADER_H_
#define OPENGLFRAMEWORK_CORE_SHADER_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <vector>
#include <string_view>

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

    void SetBool(const std::string_view name, const bool value) const {
        glUniform1i(glGetUniformLocation(shaderID_, name.data()), (int)value);
    }

    void SetInt(const std::string_view name, const int value) const {
        glUniform1i(glGetUniformLocation(shaderID_, name.data()), value);
    }

    void SetFloat(const std::string_view name, const float value) const {
        glUniform1f(glGetUniformLocation(shaderID_, name.data()), value);
    }

    void SetVec2(const std::string_view name, const glm::vec2 value) const {
        glUniform2fv(glGetUniformLocation(shaderID_, name.data()), 1, &value[0]);
    }
    void SetVec2(const std::string_view name, const float x, const float y) const {
        glUniform2f(glGetUniformLocation(shaderID_, name.data()), x, y);
    }

    void SetVec3(const std::string_view name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(shaderID_, name.data()), 1, &value[0]);
    }
    void SetVec3(const std::string_view name, const float x, const float y, 
        const float z) const 
    {
        glUniform3f(glGetUniformLocation(shaderID_, name.data()), x, y, z);
    }

    void SetVec4(const std::string_view name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(shaderID_, name.data()), 1, &value[0]);
    }
    void SetVec4(const std::string_view name, const float x, const float y,
        const float z, const float w) const 
    {
        glUniform4f(glGetUniformLocation(shaderID_, name.data()), x, y, z, w);
    }

    void SetMat2(const std::string_view name, const glm::mat2& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(shaderID_, name.data()), 1, 
            GL_FALSE, &mat[0][0]);
    }

    void SetMat3(const std::string_view name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(shaderID_, name.data()), 1, 
            GL_FALSE, &mat[0][0]);
    }

    void SetMat4(const std::string_view name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(shaderID_, name.data()), 1, 
            GL_FALSE, &mat[0][0]);
    }

private:
    std::vector<GLuint> shaders_;
    // Here shaderID actually means OpenGL's program.
    GLuint shaderID_;
    void CompileShader_(std::string_view shaderContent, const GLenum shaderType);
    void LinkShader_();
    void ClearShader_();
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_SHADER_H_
