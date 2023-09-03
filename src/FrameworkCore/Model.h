#pragma once

#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Transform.h"
#include "Framebuffer.h"

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <filesystem>
#include <stack>

namespace OpenGLFramework::Core 
{

template<typename T, typename ...Args>
void LoadResourcesDecorator_(const aiScene* model, T processFunc, Args&&... args)
{
    aiNode* node = nullptr;
    std::stack<aiNode*> childNodes;
    childNodes.push(model->mRootNode);
    while (!childNodes.empty())
    {
        node = childNodes.top();
        for (unsigned int i = 0u, len = node->mNumMeshes; i < len; i++)
        {
            aiMesh* mesh = model->mMeshes[node->mMeshes[i]];
            processFunc(mesh, std::forward<Args>(args)...);
        }
        childNodes.pop();
        for (unsigned int i = 0u, len = node->mNumChildren; i < len; i++)
        {
            childNodes.push(node->mChildren[i]);
        }
    }
};

class BasicTriModel
{
public:
    BasicTriModel(const std::filesystem::path& modelPath);
    BasicTriModel(std::vector<BasicTriMesh> init_meshes);
    std::vector<BasicTriMesh> meshes;
    Transform transform;
private:
    void LoadResources_(const aiScene* model);
};

class BasicTriRenderModel
{
    static inline GLHelper::NaiveVACFactory<BasicVertAttribContainer> defaultFactory_{};
public:
    std::vector<BasicTriRenderMesh> meshes;
    Transform transform;

    BasicTriRenderModel(std::vector<BasicTriRenderMesh> init_meshes);
    BasicTriRenderModel(const std::filesystem::path& modelPath,
        GLHelper::IVertexAttibContainerFactory & = defaultFactory_,
        bool needTBN = false, bool textureNeedFlip = false);
    // We regard std::unordered_map as nothrow move constructible.
    BasicTriRenderModel(BasicTriRenderModel&&) noexcept = default;

    void AttachTexture(const std::filesystem::path& path,
        std::initializer_list<int> attachIDs, bool isSpecular = false);
    void Draw(const Shader& shader) const;
    void Draw(const Shader& shader, const Framebuffer& buffer) const;
    void Draw(const Shader& shader,
        const std::function<void(int, const Shader&)>& preprocess,
        const std::function<void(void)>& postprocess) const;
    void Draw(const Shader& shader, const Framebuffer& buffer, 
        const std::function<void(int, const Shader&)>& preprocess,
        const std::function<void(void)>& postprocess) const;
private:
    void LoadResources_(const aiScene* model, const std::filesystem::path& 
        resourceRootPath, bool textureNeedFlip,
        GLHelper::IVertexAttibContainerFactory&);
    TexturePool texturePool_;
};

} // namespace OpenGLFramework::Core
