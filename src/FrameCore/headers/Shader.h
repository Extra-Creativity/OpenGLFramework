#pragma once
#ifndef _OPENGLFRAMEWORK_SHADER_H_
#define _OPENGLFRAMEWORK_SHADER_H_

#include <glad/glad.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

class Shader
{
public:
    Shader(const std::filesystem::path& vertexShaderFilePath, const std::filesystem::path& fragmentShaderPath);
    Shader(const std::filesystem::path& vertexShaderFilePath, const std::filesystem::path& geometryShaderPath, 
        const std::filesystem::path& fragmentShaderPath);
    ~Shader();
    GLuint shaderID;

    void Activate() { glUseProgram(shaderID); };

    void SetBool(const std::string_view name, bool value) const {
        glUniform1i(glGetUniformLocation(shaderID, name.data()), (int)value);
    }

    void SetInt(const std::string_view name, int value) const {
        glUniform1i(glGetUniformLocation(shaderID, name.data()), value);
    }

    void SetFloat(const std::string_view name, float value) const {
        glUniform1f(glGetUniformLocation(shaderID, name.data()), value);
    }

    void SetVec2(const std::string_view name, const glm::vec2 value) const {
        glUniform2fv(glGetUniformLocation(shaderID, name.data()), 1, &value[0]);
    }
    void SetVec2(const std::string_view name, float x, float y) const {
        glUniform2f(glGetUniformLocation(shaderID, name.data()), x, y);
    }

    void SetVec3(const std::string_view name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(shaderID, name.data()), 1, &value[0]);
    }
    void SetVec3(const std::string_view name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(shaderID, name.data()), x, y, z);
    }

    void SetVec4(const std::string_view name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(shaderID, name.data()), 1, &value[0]);
    }
    void SetVec4(const std::string_view name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(shaderID, name.data()), x, y, z, w);
    }

    void SetMat2(const std::string_view name, const glm::mat2& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(shaderID, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

    void SetMat3(const std::string_view name, const glm::mat3& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(shaderID, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

    void SetMat4(const std::string_view name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(shaderID, name.data()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    std::vector<GLuint> m_shaders;
    void CompileShader_(const std::string& shaderContent, GLenum shaderType);
    void LinkShader_();
    void ClearShader_();
};

#endif // !_OPENGLFRAMEWORK_SHADER_H_
