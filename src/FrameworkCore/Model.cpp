#include "Model.h"
#include "Utility/IO/IOExtension.h"

#include <stb_image.h>

#include <execution>
#include <ranges>

namespace OpenGLFramework::Core
{

BasicTriModel::BasicTriModel(const std::filesystem::path& modelPath)
{
    Assimp::Importer importer;
    const aiScene* model = importer.ReadFile(modelPath.string(),
        aiProcess_Triangulate | aiProcess_FlipUVs
        | aiProcess_JoinIdenticalVertices);
    if (model == nullptr || (model->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 ||
        model->mRootNode == nullptr) [[unlikely]]
    {
        IOExtension::LogError(importer.GetErrorString());
        return;
    }
    LoadResources_(model);
    return;
}

BasicTriModel::BasicTriModel(std::vector<BasicTriMesh> init_meshes):
    meshes{std::move(init_meshes)}
{};

void BasicTriModel::LoadResources_(const aiScene* model)
{
    LoadResourcesDecorator_(model,
        [this](const aiMesh* mesh)
        {
            // construct triangle mesh.
            this->meshes.emplace_back(mesh);
        });
    return;
}

void BasicTriRenderModel::LoadResources_(const aiScene* model, 
    const std::filesystem::path& resourceRootPath, bool textureNeedFlip,
    const GLHelper::IVertexAttribContainer& container)
{
    stbi_set_flip_vertically_on_load(textureNeedFlip);

    LoadResourcesDecorator_(model, 
        [this](const aiMesh* mesh, const aiScene* model,
            const std::filesystem::path& resourceRootPath,
            const GLHelper::IVertexAttribContainer& c) 
        {
            aiMaterial* material = model->mMaterials[mesh->mMaterialIndex];
            // construct triangle mesh.
            this->meshes.emplace_back(mesh, material, this->texturePool_, 
                resourceRootPath, c);
        }, model, resourceRootPath, container);

    stbi_set_flip_vertically_on_load(false);
}

BasicTriRenderModel::BasicTriRenderModel(std::vector<BasicTriRenderMesh> 
    init_meshes) : meshes{ std::move(init_meshes) }
{ };

BasicTriRenderModel::BasicTriRenderModel(const std::filesystem::path& modelPath, 
    const GLHelper::IVertexAttribContainer& container, bool textureNeedFlip)
{
    Assimp::Importer importer;
    const aiScene* model = importer.ReadFile(modelPath.string(), 
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
        | aiProcess_JoinIdenticalVertices);
    if (model == nullptr || (model->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 ||
        model->mRootNode == nullptr) [[unlikely]]
    {
        IOExtension::LogError(importer.GetErrorString());
        return;
    }
    // NOTICE that we assume the model is at the root path.
    const std::filesystem::path resourceRootPath = modelPath.parent_path();
    LoadResources_(model, resourceRootPath, textureNeedFlip, container);
    return;
}

void BasicTriRenderModel::AttachTexture(const std::filesystem::path& path,
    std::initializer_list<int> attachIDs, bool isSpecular)
{
    auto [textureIt, _] = texturePool_.try_emplace(path, path);
    if (isSpecular)
    {
        for (auto attachID : attachIDs)
            meshes.at(attachID).specularTextureRefs_.push_back(textureIt->second);
        return;
    }
    // else, diffuse.
    for (auto attachID : attachIDs)
        meshes.at(attachID).diffuseTextureRefs_.push_back(textureIt->second);
    return;
};

void BasicTriRenderModel::Draw(const Shader& shader) const
{
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }
    return;
}

void BasicTriRenderModel::Draw(const Shader& shader, 
    const Framebuffer& buffer) const
{
    buffer.UseAsRenderTarget();
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }
    Framebuffer::RestoreDefaultRenderTarget();
    return;
};

void BasicTriRenderModel::Draw(const Shader& shader, 
    const std::function<void(int, const Shader&)>& preprocess,
    const std::function<void(void)>& postprocess) const
{
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader, preprocess, postprocess);
    }
    return;
};

void BasicTriRenderModel::Draw(const Shader& shader, const Framebuffer& buffer,
    const std::function<void(int, const Shader&)>& preprocess,
    const std::function<void(void)>& postprocess) const
{
    buffer.UseAsRenderTarget();
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader, preprocess, postprocess);
    }
    Framebuffer::RestoreDefaultRenderTarget();
};

} // namespace OpenGLFramework::Core