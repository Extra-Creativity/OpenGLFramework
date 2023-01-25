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

void BasicRenderTriMesh::AllocateAndMemcpyVerticesData_()
{
    size_t elementSize = sizeof(glm::vec3) + sizeof(VertexAttribute);
    // allocate memory.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * elementSize, nullptr, 
        GL_STATIC_DRAW);

    size_t verticeSizeOffset = sizeof(glm::vec3) * vertices.size();
    // memcpy vertex position to GPU.
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticeSizeOffset, vertices.data());
    // memcpy other attributes to GPU.
    glBufferSubData(GL_ARRAY_BUFFER, verticeSizeOffset, 
        verticesAttributes.size() * sizeof(VertexAttribute), 
        verticesAttributes.data());
    return;
}

void BasicRenderTriMesh::BindVerticesAttributeSequence_()
{
    size_t verticeSizeOffset = sizeof(glm::vec3) * vertices.size();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    size_t normalAttributeBeginOffset = verticeSizeOffset + 
        offsetof(VertexAttribute, normalCoord);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), 
        reinterpret_cast<void*>(normalAttributeBeginOffset));

    size_t textureAttributeBeginOffset = verticeSizeOffset +
        offsetof(VertexAttribute, textureCoord);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), 
        reinterpret_cast<void*>(textureAttributeBeginOffset));
    return;
}

void BasicRenderTriMesh::AllocateAndMemcpyTrianglesData_()
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::ivec3),
        triangles.data(), GL_STATIC_DRAW);
    return;
};

void BasicRenderTriMesh::SetupRenderResource_()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    AllocateAndMemcpyVerticesData_();
    BindVerticesAttributeSequence_();

    glGenBuffers(1, &IBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    AllocateAndMemcpyTrianglesData_();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    std::cout << VAO << " " << VBO << " " << IBO << "\n";
}

void BasicRenderTriMesh::CopyAttributes_(const aiMesh* mesh)
{
    std::ranges::iota_view vertexIDView{ size_t(0), vertices.size() };
    std::for_each(vertexIDView.begin(), vertexIDView.end(),
        [mesh, this](size_t id) {
            auto& dstVertAttribute = verticesAttributes[id];
            auto srcNorm = mesh->mNormals[id];
            dstVertAttribute.normalCoord = { srcNorm.x,
                srcNorm.y, srcNorm.z };

            if (auto srcTextureCoords = mesh->mTextureCoords[0])
            {
                auto& srcTextureCoord = srcTextureCoords[id];
                dstVertAttribute.textureCoord = { srcTextureCoord.x,
                    srcTextureCoord.y };
            }
            else
                dstVertAttribute.textureCoord = { 0.f, 0.f };
            return;
        });
    return;
}

void BasicRenderTriMesh::AddTexturesToPoolAndFillRefsByType_(
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

void BasicRenderTriMesh::AddAllTexturesToPoolAndFillRefs_(
    const aiMaterial* material, TexturePool& texturePool,
    const std::filesystem::path& rootPath)
{
    AddTexturesToPoolAndFillRefsByType_(material, aiTextureType_DIFFUSE,
        diffuseTextures, texturePool, rootPath);
    AddTexturesToPoolAndFillRefsByType_(material, aiTextureType_SPECULAR,
        specularTextures, texturePool, rootPath);
}

BasicRenderTriMesh::BasicRenderTriMesh(const aiMesh* mesh,
    const aiMaterial* material, TexturePool& texturePool,
    const std::filesystem::path& rootPath):
    BasicTriMesh{mesh}, verticesAttributes(mesh->mNumVertices)
{
    CopyAttributes_(mesh);
    SetupRenderResource_();
    AddAllTexturesToPoolAndFillRefs_(material, texturePool, rootPath);
    return;
};

BasicRenderTriMesh::~BasicRenderTriMesh()
{
    //glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &IBO);
    //glDeleteVertexArrays(1, &VAO);
}

void BasicRenderTriMesh::Draw(Shader& shader)
{
    int textureCnt = 0;
    auto SetTexture = [&textureCnt, &shader](const std::string& namePrefix, decltype(diffuseTextures) textures) 
    {
        int currTypeID = 1;
        for (auto& texture : textures)
        {
            glActiveTexture(GL_TEXTURE0 + textureCnt);
            shader.SetInt(namePrefix + std::to_string(currTypeID), textureCnt);
            glBindTexture(GL_TEXTURE_2D, texture.get().ID);
            ++textureCnt, ++currTypeID;
        }
        return;
    };
    SetTexture("diffuseTexture", diffuseTextures);
    SetTexture("specularTexture", specularTextures);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(triangles.size() * 3), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return;
};

void BasicRenderTriMesh::Draw(Shader& shader, Framebuffer& buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    if (buffer.renderBuffer != 0)
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    auto& color = buffer.backgroundColor;
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);

    Draw(shader);

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
};

}