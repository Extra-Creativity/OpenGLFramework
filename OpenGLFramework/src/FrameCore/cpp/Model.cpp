#include "Model.h"
#include "IOExtension.h"

#include <stb_image.h>
#include <stack>
#include <execution>
#include <fstream>

#include "config.h"
#include "GPUExtension.h"

void Mesh::SetupMesh_()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(glm::ivec3), triangles.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normalCoord)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, textureCoord)));

    glBindVertexArray(0);
}

Mesh::Mesh(aiMesh* mesh, const aiScene* model) : vertices(mesh->mNumVertices), triangles(mesh->mNumFaces)
{
#if OPENGLFRAMEWORK_ENABLE_GPUEXTENSION
    unsigned int vertNum = mesh->mNumVertices;
    GPUExtension::GPUvector srcVerts{mesh->mVertices, vertNum},
        srcNorms{mesh->mNormals, vertNum}, srcTextures{mesh->mTextureCoords[0], vertNum};
    GPUExtension::GPUExecuteConfig config{{1024, 1, 1}, {1 + vertNum / 1024, 1, 1}, false};
    GPUExtension::GPUvector<Vertex> dstVerts{vertNum};
    GPUExtension::Mesh::LoadVertices(config, dstVerts, srcVerts, srcNorms, srcTextures);
#else
    std::for_each(/*std::execution::par_unseq,*/ vertices.begin(), vertices.end(),
        [mesh, vertBegin = vertices.data()](Vertex& currVert){
        ptrdiff_t pos = &currVert - vertBegin;
        auto srcVerts = mesh->mVertices;
        currVert.position.x = srcVerts[pos].x;
        currVert.position.y = srcVerts[pos].y;
        currVert.position.z = srcVerts[pos].z;

        if (mesh->HasNormals())
        {
            auto srcNorms = mesh->mNormals;
            currVert.normalCoord.x = srcNorms[pos].x;
            currVert.normalCoord.y = srcNorms[pos].y;
            currVert.normalCoord.z = srcNorms[pos].z;
        }
        else
            currVert.normalCoord = { 0.f, 0.f, 0.f };

        if (auto srcTextures = mesh->mTextureCoords[0])
        {
            currVert.textureCoord.x = srcTextures[pos].x;
            currVert.textureCoord.y = srcTextures[pos].y;
        }
        else
            currVert.textureCoord = { 0.f, 0.f };
        return;
    });
#endif // OPENGLFRAMEWORK_ENABLE_GPUEXTENSION != 0

    // for aiFace is pretty small, we do not use reference.
    if (!mesh->HasNormals())
    {
#       if OPENGLFRAMEWORK_ENABLE_GPUEXTENSION
        GPUExtension::GPUSynchronize();
        dstVerts.ToCPUVector(vertices.data());
#       endif

        std::for_each(std::execution::par_unseq, triangles.begin(), triangles.end(),
            [facets = mesh->mFaces, triBegin = triangles.data(), this](glm::ivec3& currTri)
        {
            ptrdiff_t pos = &currTri - triBegin;
            aiFace currFacet = facets[pos];
            auto v1 = currFacet.mIndices[0], v2 = currFacet.mIndices[1], v3 = currFacet.mIndices[2];
            auto e1 = vertices[v1].position - vertices[v2].position,
                e2 = vertices[v2].position - vertices[v3].position;
            auto norm = glm::normalize(glm::cross(e1, e2));
            vertices[v1].normalCoord += norm;
            vertices[v2].normalCoord += norm;
            vertices[v3].normalCoord += norm;
            currTri = { v1,v2,v3 };
        });

        std::for_each(std::execution::par_unseq, vertices.begin(), vertices.end(),
            [](Vertex& currVert) { currVert.normalCoord = glm::normalize(currVert.normalCoord);  });
    }
    else
    {
        std::for_each(std::execution::par_unseq, triangles.begin(), triangles.end(),
            [facets = mesh->mFaces, triBegin = triangles.data()](glm::ivec3& currTri)
        {
            ptrdiff_t pos = &currTri - triBegin;
            aiFace currFacet = facets[pos];
            currTri = { currFacet.mIndices[0], currFacet.mIndices[1], currFacet.mIndices[2] };
        });

#       if OPENGLFRAMEWORK_ENABLE_GPUEXTENSION
            GPUExtension::GPUSynchronize();
            dstVerts.ToCPUVector(vertices.data());
#       endif

    }
    SetupMesh_();
    return;
};

void Mesh::Draw(Shader& shader)
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
    glDrawElements(GL_TRIANGLES, triangles.size() * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    return;
};

void Mesh::Draw(Shader& shader, Framebuffer& buffer)
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

void Model::LoadTextureWithType_(const std::filesystem::path& resourcePath, aiMaterial* material, aiTextureType type, 
    std::vector<std::reference_wrapper<Texture>>& meshTextures)
{
    unsigned int len = material->GetTextureCount(type);
    meshTextures.reserve(len);
    for (unsigned int i = 0u; i < len; i++)
    {
        aiString pathStr;
        material->GetTexture(type, i, &pathStr);
        const char8_t* u8Path = reinterpret_cast<const char8_t*>(pathStr.C_Str());
        // to make it absolute path so that absolute & relative path will be seen as one.
        std::filesystem::path path = std::filesystem::canonical(resourcePath / u8Path);
        auto it = textures.find(path);
        if (it == textures.end())
        {
            auto pos = textures.emplace(path, path); // one used for key, the other used for Texture initialization.
            meshTextures.push_back(std::ref(pos.first->second));
        }
        else
        {
            meshTextures.push_back(std::ref(it->second));
        }
    }
    return;
}

void Model::TransferTextureData_(const std::filesystem::path& resourcePath, aiMaterial* material, Mesh& fillMesh)
{
    LoadTextureWithType_(resourcePath, material, aiTextureType_DIFFUSE, fillMesh.diffuseTextures);
    LoadTextureWithType_(resourcePath, material, aiTextureType_SPECULAR, fillMesh.specularTextures);
    return;
}

Model::Model(const std::filesystem::path& modelPath, bool textureNeedFlip)
{
    stbi_set_flip_vertically_on_load(textureNeedFlip);
    Assimp::Importer importer;
    const aiScene* model = importer.ReadFile(modelPath.string(), aiProcess_Triangulate 
        | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (model == nullptr || (model->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 ||
        model->mRootNode == nullptr) [[unlikely]]
    {
        IOExtension::LogError(std::source_location::current(), importer.GetErrorString());
        return;
    }
    
    aiNode* node = nullptr;
    std::stack<aiNode*> childNodes;
    childNodes.push(model->mRootNode);
    const std::filesystem::path resourcePath = modelPath.parent_path();
    while (!childNodes.empty())
    {
        node = childNodes.top();
        for (unsigned int i = 0u, len = node->mNumMeshes; i < len; i++)
        {
            aiMesh* mesh = model->mMeshes[node->mMeshes[i]];
            auto& pos = meshes.emplace_back(mesh, model);
            aiMaterial* material = model->mMaterials[mesh->mMaterialIndex];
            TransferTextureData_(resourcePath, material, *pos);
        }
        childNodes.pop();
        for (unsigned int i = 0u, len = node->mNumChildren; i < len; i++)
        {
            childNodes.push(node->mChildren[i]);
        }
    } 
    stbi_set_flip_vertically_on_load(false);
    // RAII: once the importer destructs, the model resource will be released.
    return;
}

void Model::Draw(Shader& shader)
{
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }
    return;
}

void Model::Draw(Shader& shader, Framebuffer& buffer)
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

    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
};
