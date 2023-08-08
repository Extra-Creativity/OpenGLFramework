#pragma once
#ifndef OPENGLFRAMEWORK_CORE_MESH_H_
#define OPENGLFRAMEWORK_CORE_MESH_H_

#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 26451 26812)
#endif

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <unordered_map>
#include <filesystem>
#include <functional>
#include <vector>
#include <array>

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
    using TriangleVerts = std::array<std::reference_wrapper<glm::vec3>, 3>;
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> triangles;
    BasicTriMesh(const aiMesh* mesh);
    BasicTriMesh(std::vector<glm::vec3> init_vertices, 
        std::vector<glm::ivec3> init_triangles);
    TriangleVerts  GetTriangleVerts(size_t index);
    TriangleVerts GetTriangleVerts(glm::ivec3 triangle);
    std::vector<glm::vec3> GetRealTriNormals();
    std::vector<glm::vec3> GetRealVertexNormals();
};

class BasicTriRenderMesh : public BasicTriMesh
{
public:
    std::vector<VertexAttribute> verticesAttributes;
    std::vector<std::reference_wrapper<Texture>> diffuseTextureRefs;
    std::vector<std::reference_wrapper<Texture>> specularTextureRefs;

    BasicTriRenderMesh(BasicTriMesh mesh, 
        const std::vector<glm::vec3>& init_normals);
    BasicTriRenderMesh(const aiMesh* mesh, const aiMaterial* material, 
        TexturePool& texturePool, const std::filesystem::path& rootPath);
    BasicTriRenderMesh(const BasicTriRenderMesh&) = delete;
    BasicTriRenderMesh& operator=(const BasicTriRenderMesh&) = delete;
    BasicTriRenderMesh(BasicTriRenderMesh&&) noexcept;
    BasicTriRenderMesh& operator=(BasicTriRenderMesh&&) noexcept;
    ~BasicTriRenderMesh();

    void Draw(Shader& shader);
    void Draw(Shader& shader, Framebuffer& buffer);
    void Draw(Shader& shader, const std::function<void(int, Shader&)>& preprocess,
        const std::function<void(void)>& postprocess);
    void Draw(Shader& shader, Framebuffer& buffer, 
        const std::function<void(int, Shader&)>& preprocess,
        const std::function<void(void)>& postprocess);
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

    void SetTextures_(Core::Shader& shader, const std::string& namePrefix,
        const decltype(diffuseTextureRefs)& textures, int& beginID);
};

}

#endif // !OPENGLFRAMEWORK_CORE_MESH_H_