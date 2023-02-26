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
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
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
    const std::filesystem::path& resourceRootPath, bool textureNeedFlip)
{
    stbi_set_flip_vertically_on_load(textureNeedFlip);

    LoadResourcesDecorator_(model, 
        [this](const aiMesh* mesh, const aiScene* model,
        const std::filesystem::path& resourceRootPath) 
        {
            aiMaterial* material = model->mMaterials[mesh->mMaterialIndex];
            // construct triangle mesh.
            this->meshes.emplace_back(mesh, material, this->texturePool, 
                resourceRootPath);
        }, model, resourceRootPath);

    stbi_set_flip_vertically_on_load(false);
}

BasicTriRenderModel::BasicTriRenderModel(const std::filesystem::path& modelPath, 
    bool textureNeedFlip)
{
    Assimp::Importer importer;
    const aiScene* model = importer.ReadFile(modelPath.string(), 
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    if (model == nullptr || (model->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0 ||
        model->mRootNode == nullptr) [[unlikely]]
    {
        IOExtension::LogError(importer.GetErrorString());
        return;
    }
    // NOTICE that we assume the model is at the root path.
    const std::filesystem::path resourceRootPath = modelPath.parent_path();
    LoadResources_(model, resourceRootPath, textureNeedFlip);
    return;
}

void BasicTriRenderModel::InitiateFramebufferContext_(Framebuffer& buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, buffer.GetFramebuffer());
    if (buffer.NeedDepthTesting())
    {
        glEnable(GL_DEPTH_TEST);
        if(buffer.needDepthClear)
            glClear(GL_DEPTH_BUFFER_BIT);
    }
    auto& color = buffer.backgroundColor;
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    return;
};

void BasicTriRenderModel::EndFramebufferContext_()
{
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return;
};

void BasicTriRenderModel::Draw(Shader& shader)
{
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }
    return;
}

void BasicTriRenderModel::Draw(Shader& shader, Framebuffer& buffer)
{
    InitiateFramebufferContext_(buffer);
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader);
    }
    EndFramebufferContext_();
    return;
};

void BasicTriRenderModel::Draw(Shader& shader, 
    const std::function<void(int, Shader&)>& preprocess,
    const std::function<void(void)>& postprocess)
{
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader, preprocess, postprocess);
    }
    return;
};

void BasicTriRenderModel::Draw(Shader& shader, Framebuffer& buffer,
    const std::function<void(int, Shader&)>& preprocess,
    const std::function<void(void)>& postprocess)
{
    InitiateFramebufferContext_(buffer);
    for (auto& mesh : meshes)
    {
        mesh.Draw(shader, preprocess, postprocess);
    }
    EndFramebufferContext_();
};

} // namespace OpenGLFramework::Core