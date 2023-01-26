#pragma once
#ifndef OPENGLFRAMEWORK_CORE_MESH_H_
#define OPENGLFRAMEWORK_CORE_MESH_H_

#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <filesystem>
#include <functional>
#include <vector>

namespace OpenGLFramework::Core
{
struct PathHash_AssumeCanonical {
    size_t operator()(const std::filesystem::path& path) const {
        return std::filesystem::hash_value(path);
    }
};
// Key should be absolute path so that absolute & relative path will be seen as one.
using TexturePool = std::unordered_map<std::filesystem::path, Texture,
    PathHash_AssumeCanonical>;

struct VertexAttribute
{
    glm::vec3 normalCoord;
    glm::vec2 textureCoord;
};

class BasicTriMesh
{
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> triangles;
    BasicTriMesh(const aiMesh* mesh);
    std::vector<glm::vec3> GetRealTriNormals();
    std::vector<glm::vec3> GetRealVertexNormals();
};

class BasicRenderTriMesh : public BasicTriMesh
{
public:
    std::vector<VertexAttribute> verticesAttributes;
    std::vector<std::reference_wrapper<Texture>> diffuseTextureRefs;
    std::vector<std::reference_wrapper<Texture>> specularTextureRefs;

    BasicRenderTriMesh(const aiMesh* mesh, const aiMaterial* material, 
        TexturePool& texturePool, const std::filesystem::path& rootPath);
    BasicRenderTriMesh(const BasicRenderTriMesh&) = delete;
    BasicRenderTriMesh& operator=(const BasicRenderTriMesh&) = delete;
    BasicRenderTriMesh(BasicRenderTriMesh&&) noexcept;
    BasicRenderTriMesh& operator=(BasicRenderTriMesh&&) noexcept;
    ~BasicRenderTriMesh();

    void Draw(Shader& shader);
    void Draw(Shader& shader, Framebuffer& buffer);
private:
    GLuint VAO, VBO, IBO;
    void AllocateAndMemcpyVerticesData_();
    void BindVerticesAttributeSequence_();
    void AllocateAndMemcpyTrianglesData_();
    void ReleaseRenderResources_();

    void SetupRenderResource_();
    void CopyAttributes_(const aiMesh* mesh);
    void AddAllTexturesToPoolAndFillRefs_(const aiMaterial* material,
        TexturePool& texturePool, const std::filesystem::path& rootPath);
    void AddTexturesToPoolAndFillRefsByType_(const aiMaterial* material,
        const aiTextureType type, std::vector<std::reference_wrapper<Texture>>& refs,
        TexturePool& texturePool, const std::filesystem::path& rootPath);
};

}

#endif // !OPENGLFRAMEWORK_CORE_MESH_H_