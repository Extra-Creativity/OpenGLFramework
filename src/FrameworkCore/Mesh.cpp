#include "Mesh.h"

#include <ranges>
#include <iostream>

namespace OpenGLFramework::Core
{

BasicTriMesh::BasicTriMesh(const aiMesh* mesh): vertices(mesh->mNumVertices),
    triangles(mesh->mNumFaces)
{
    std::ranges::iota_view vertexIDView{ size_t(0), vertices.size() };
    std::for_each(vertexIDView.begin(), vertexIDView.end(),
        [mesh, this](size_t id) {
            auto srcVert = mesh->mVertices[id];
            auto& dstVert = vertices[id];
            dstVert = { srcVert.x, srcVert.y, srcVert.z };
        });

    std::ranges::iota_view triangleIDView{ size_t(0), triangles.size() };
    std::for_each(triangleIDView.begin(), triangleIDView.end(),
        [mesh, this](size_t id)
        {
            aiFace currFacet = mesh->mFaces[id];
            triangles[id] = { currFacet.mIndices[0], currFacet.mIndices[1], 
                currFacet.mIndices[2] };
        });
    return;
}

BasicTriMesh::BasicTriMesh(std::vector<glm::vec3> init_vertices,
    std::vector<glm::ivec3> init_triangles) : 
    vertices{std::move(init_vertices)}, triangles{std::move(init_triangles)}
{};

BasicTriMesh::TriangleVerts BasicTriMesh::GetTriangleVerts(size_t index)
{
    auto& triangle = triangles.at(index);
    return GetTriangleVerts(triangle);
};

BasicTriMesh::TriangleVerts BasicTriMesh::GetTriangleVerts(glm::ivec3 triangle)
{
    return { vertices.at(triangle.x), vertices.at(triangle.y),
        vertices.at(triangle.z) };
};

std::vector<glm::vec3> BasicTriMesh::GetRealTriNormals()
{
    std::vector<glm::vec3> normals(triangles.size());
    std::ranges::iota_view triangleView{size_t(0), triangles.size()};
    std::for_each(triangleView.begin(), triangleView.end(),
        [&normals, this](size_t id)
        {
            auto v1 = triangles[id][0], v2 = triangles[id][1],
                v3 = triangles[id][2];
            glm::vec3 e1 = vertices[v1] - vertices[v2],
                e2 = vertices[v2] - vertices[v3];
            normals[id] = glm::normalize(glm::cross(e1, e2));
        });
    return normals;
}

std::vector<glm::vec3> BasicTriMesh::GetRealVertexNormals()
{
    std::vector<glm::vec3> normals(vertices.size());
    std::ranges::iota_view triangleView{ size_t(0), triangles.size() };
    std::for_each(triangleView.begin(), triangleView.end(),
        [&normals, this](size_t id)
        {
            auto v1 = triangles[id][0], v2 = triangles[id][1],
                v3 = triangles[id][2];
            glm::vec3 e1 = vertices[v1] - vertices[v2],
                e2 = vertices[v2] - vertices[v3];
            auto norm = glm::normalize(glm::cross(e1, e2));
            // NOTICE : race if parallel!
            normals[v1] += norm, normals[v2] += norm, normals[v3] += norm;
        });
    std::for_each(normals.begin(), normals.end(), [](glm::vec3& vec) 
        {
            vec = glm::normalize(vec);
        });
    return normals;
};

void BasicTriRenderMesh::SetupRenderResource_()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    verticesAttributes_->AllocateAndBind(sizeof(glm::vec3), vertices.size());
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertices.size(),
        vertices.data());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    glGenBuffers(1, &IBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::ivec3),
        triangles.data(), GL_STATIC_DRAW);

    // NOTICE: Unbind sequence MUST be VAO->VBO&IBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BasicTriRenderMesh::CopyAttributes_(const aiMesh* mesh)
{
    verticesAttributes_->CopyFromMesh(mesh);
    return;
}

void BasicTriRenderMesh::AddTexturesToPoolAndFillRefsByType_(
    const aiMaterial* material, const aiTextureType type, 
    std::vector<std::reference_wrapper<Texture>>& refs, 
    TexturePool& texturePool, const std::filesystem::path& rootPath)
{
    unsigned int len = material->GetTextureCount(type);
    refs.reserve(len);
    for (unsigned int i = 0u; i < len; i++)
    {
        aiString pathStr;
        material->GetTexture(type, i, &pathStr);
        const char8_t* u8Path = reinterpret_cast<const char8_t*>(pathStr.C_Str());
        std::filesystem::path path = std::filesystem::canonical(rootPath / u8Path);
        // Insert {path, Texture}, and Texture is also constructed from path.
        // NOTICE that we use try_emplace to avoid Texture construction if it
        // already exists in the pool!
        auto [textureIt, _] = texturePool.try_emplace(path, path);
        refs.push_back(std::ref(textureIt->second));
    }
    return;
}

void BasicTriRenderMesh::AddAllTexturesToPoolAndFillRefs_(
    const aiMaterial* material, TexturePool& texturePool,
    const std::filesystem::path& rootPath)
{
    AddTexturesToPoolAndFillRefsByType_(material, aiTextureType_DIFFUSE,
        diffuseTextureRefs_, texturePool, rootPath);
    AddTexturesToPoolAndFillRefsByType_(material, aiTextureType_SPECULAR,
        specularTextureRefs_, texturePool, rootPath);
}

BasicTriRenderMesh::BasicTriRenderMesh(const aiMesh* mesh,
    const aiMaterial* material, TexturePool& texturePool,
    const std::filesystem::path& rootPath,
    std::unique_ptr<GLHelper::IVertexAttribContainer> ptr):
    BasicTriMesh{ mesh }, verticesAttributes_{ std::move(ptr) }
{
    CopyAttributes_(mesh);
    SetupRenderResource_();
    AddAllTexturesToPoolAndFillRefs_(material, texturePool, rootPath);
    return;
};

BasicTriRenderMesh::BasicTriRenderMesh(BasicTriMesh mesh,
    const std::vector<glm::vec3>& init_normals) : 
    BasicTriMesh{ std::move(mesh) }
{
    std::vector<BasicVertexAttribute> attribs(init_normals.size());
    for (size_t i = 0; i < attribs.size(); i++)
        attribs[i].normalCoord = init_normals[i];
    verticesAttributes_ = 
        std::make_unique<BasicVertAttribContainer>(std::move(attribs));
    SetupRenderResource_();
    return;
};

BasicTriRenderMesh::BasicTriRenderMesh(BasicTriMesh mesh,
    std::vector<BasicVertexAttribute> attrs) :
    BasicTriMesh{ std::move(mesh) }, verticesAttributes_{ 
        std::make_unique<BasicVertAttribContainer>(std::move(attrs))
    }
{
    SetupRenderResource_();
    return;
};

void BasicTriRenderMesh::ReleaseRenderResources_()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
    glDeleteVertexArrays(1, &VAO);
    return;
}

BasicTriRenderMesh::BasicTriRenderMesh(BasicTriRenderMesh&& another) noexcept:
    BasicTriMesh{ std::move(another) }, 
    verticesAttributes_{ std::move(another.verticesAttributes_)},
    diffuseTextureRefs_{ std::move(another.diffuseTextureRefs_)},
    specularTextureRefs_{ std::move(another.specularTextureRefs_)},
    VAO{ another.VAO }, VBO{ another.VBO }, IBO{ another.IBO }
{
    another.VAO = another.VBO = another.IBO = 0;
    return;
}

BasicTriRenderMesh& BasicTriRenderMesh::operator=(BasicTriRenderMesh&& another)
    noexcept
{
    if (&another == this) [[unlikely]]
        return *this;
    ReleaseRenderResources_();

    BasicTriMesh::operator=(std::move(another));
    verticesAttributes_ = std::move(another.verticesAttributes_);
    diffuseTextureRefs_ = std::move(another.diffuseTextureRefs_);
    specularTextureRefs_ = std::move(another.specularTextureRefs_);

    VAO = another.VAO, VBO = another.VBO, IBO = another.IBO;
    another.VAO = another.VBO = another.IBO = 0;
    return *this;
}

BasicTriRenderMesh::~BasicTriRenderMesh()
{
    ReleaseRenderResources_();
}

void BasicTriRenderMesh::SetTextures_(const Shader& shader,
    const std::string& namePrefix,
    const decltype(diffuseTextureRefs_)& textures, int& beginID) const
{
    int currTypeID = 1;
    for (auto& texture : textures)
    {
        std::string name = namePrefix + std::to_string(currTypeID);
        Texture::BindTextureOnShader(beginID, name.c_str(), shader,
            texture.get().GetID());
        ++beginID, ++currTypeID;
    }
};

void BasicTriRenderMesh::Draw(const Shader& shader) const
{
    int textureCnt = 0;
    SetTextures_(shader, "diffuseTexture", diffuseTextureRefs_, textureCnt);
    SetTextures_(shader, "specularTexture", specularTextureRefs_, textureCnt);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangles.size() * 3), 
        GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return;
};

void BasicTriRenderMesh::Draw(const Shader& shader, 
    const std::function<void(int, const Shader&)>& preprocess,
    const std::function<void(void)>& postprocess) const
{
    int textureCnt = 0;
    SetTextures_(shader, "diffuseTexture", diffuseTextureRefs_, textureCnt);
    SetTextures_(shader, "specularTexture", specularTextureRefs_, textureCnt);
    if(preprocess) [[likely]]
        preprocess(textureCnt, shader);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangles.size() * 3),
        GL_UNSIGNED_INT, 0);
    if (postprocess) [[unlikely]]
        postprocess();
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return;
};

void BasicTriRenderMesh::Draw(const Shader& shader, const Framebuffer& buffer) const
{
    buffer.UseAsRenderTarget();
    Draw(shader);
    Framebuffer::RestoreDefaultRenderTarget();
    return;
};

void BasicTriRenderMesh::Draw(const Shader& shader, const Framebuffer& buffer,
    const std::function<void(int, const Shader&)>& preprocess,
    const std::function<void(void)>& postprocess) const
{
    buffer.UseAsRenderTarget();
    Draw(shader, preprocess, postprocess);
    Framebuffer::RestoreDefaultRenderTarget();
    return;
};

}