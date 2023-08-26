#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "../Utility/GLHelper/VertexAttribHelper.h"

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
#include <span>

namespace OpenGLFramework::Core
{
struct BasicVertexAttribute
{
    glm::vec3 normalCoord;
    glm::vec2 textureCoord;
};

template<int N, typename T>
void CopyAiVecToGLMVec(aiVector3D& aiVec, glm::vec<N, T>& vec)
{
    constexpr int limit = std::min(N, 3);
    for (int i = 0; i < limit; i++)
        vec[i] = aiVec[i];
}

template<typename T>
void CopyBasicAttributes(std::span<T> verticesAttributes_, const aiMesh* mesh)
{
    for (size_t id = 0; id < verticesAttributes_.size(); id++)
    {
        auto& dstVertAttribute = verticesAttributes_[id];
        CopyAiVecToGLMVec(mesh->mNormals[id], dstVertAttribute.normalCoord);

        if (auto srcTextureCoords = mesh->mTextureCoords[0]) [[likely]]
            CopyAiVecToGLMVec(srcTextureCoords[id], dstVertAttribute.textureCoord);
        else
            dstVertAttribute.textureCoord = { 0.f, 0.f };
    }
}
}

BEGIN_REFLECT(OpenGLFramework::Core::BasicVertexAttribute);
REFLECT(1, float, normalCoord);
REFLECT(2, float, textureCoord);
END_REFLECT(2);

VERTEX_ATTRIB_SPECIALIZE_COPY(
    std::vector<OpenGLFramework::Core::BasicVertexAttribute>& attribs,
    const aiMesh* mesh)
{
    attribs.resize(mesh->mNumVertices);
    std::span attribSpan = attribs;
    OpenGLFramework::Core::CopyBasicAttributes(attribSpan, mesh);
}

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

class BasicTriMesh
{
    using TriangleVerts = std::array<std::reference_wrapper<glm::vec3>, 3>;
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> triangles;
    BasicTriMesh(const aiMesh* mesh);
    BasicTriMesh(std::vector<glm::vec3> init_vertices, 
        std::vector<glm::ivec3> init_triangles);
    TriangleVerts GetTriangleVerts(size_t index);
    TriangleVerts GetTriangleVerts(glm::ivec3 triangle);
    std::vector<glm::vec3> GetRealTriNormals();
    std::vector<glm::vec3> GetRealVertexNormals();
};

class BasicTriRenderMesh : public BasicTriMesh
{
    friend class BasicTriRenderModel;
public:
    BasicTriRenderMesh(BasicTriMesh mesh, 
        const std::vector<glm::vec3>& init_normals);
    BasicTriRenderMesh(BasicTriMesh mesh, std::vector<BasicVertexAttribute> attrs);
    BasicTriRenderMesh(const aiMesh* mesh, const aiMaterial* material, 
        TexturePool& texturePool, const std::filesystem::path& rootPath,
        GLHelper::IVertexAttribContainer);
    BasicTriRenderMesh(const BasicTriRenderMesh&) = delete;
    BasicTriRenderMesh& operator=(const BasicTriRenderMesh&) = delete;
    BasicTriRenderMesh(BasicTriRenderMesh&&) noexcept;
    BasicTriRenderMesh& operator=(BasicTriRenderMesh&&) noexcept;
    ~BasicTriRenderMesh();

    void Draw(const Shader& shader) const;
    void Draw(const Shader& shader, const Framebuffer& buffer) const;
    void Draw(const Shader& shader,
        const std::function<void(int, const Shader&)>& preprocess,
        const std::function<void(void)>& postprocess) const;
    void Draw(const Shader& shader, const Framebuffer& buffer, 
        const std::function<void(int, const Shader&)>& preprocess,
        const std::function<void(void)>& postprocess) const;
private:
    GLHelper::IVertexAttribContainer verticesAttributes_;
    std::vector<std::reference_wrapper<Texture>> diffuseTextureRefs_;
    std::vector<std::reference_wrapper<Texture>> specularTextureRefs_;
    GLuint VAO, VBO, IBO;

    void ReleaseRenderResources_();

    void SetupRenderResource_();
    void CopyAttributes_(const aiMesh* mesh);
    void AddAllTexturesToPoolAndFillRefs_(const aiMaterial* material,
        TexturePool& texturePool, const std::filesystem::path& rootPath);
    void AddTexturesToPoolAndFillRefsByType_(const aiMaterial* material,
        const aiTextureType type, std::vector<std::reference_wrapper<Texture>>& refs,
        TexturePool& texturePool, const std::filesystem::path& rootPath);

    void SetTextures_(const Core::Shader& shader, const std::string& namePrefix,
        const decltype(diffuseTextureRefs_)& textures, int& beginID) const;
};

}
