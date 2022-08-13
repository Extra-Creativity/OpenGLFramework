#pragma once
// To prevent conflict from macros in assimp.
#ifndef _OPENGLFRAMEWORK_MODEL_H_
#define _OPENGLFRAMEWORK_MODEL_H_

#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
#include "Framebuffer.h"

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex
{
public:
    glm::vec3 position;
    glm::vec3 normalCoord;
    glm::vec2 textureCoord;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<glm::ivec3> triangles;
    std::vector<std::reference_wrapper<Texture>> diffuseTextures;
    std::vector<std::reference_wrapper<Texture>> specularTextures;
    Mesh(aiMesh* mesh, const aiScene* model);
    void Draw(Shader& shader);
    void Draw(Shader& shader, Framebuffer& buffer);
private:
    GLuint VAO, VBO, IBO;
    void SetupMesh_();
};

class Model
{
private:
    struct PathHash {
        size_t operator()(const std::filesystem::path& path) const {
            return std::filesystem::hash_value(path);
        }
    };
public:
    std::unordered_map<std::filesystem::path, Texture, PathHash> textures;
    std::vector<Mesh> meshes;
    Transform transform;
    Model(const std::filesystem::path& modelPath, bool textureNeedFlip = false);
    void Draw(Shader& shader);
    void Draw(Shader& shader, Framebuffer& buffer);
private:
    void TransferTextureData_(const std::filesystem::path& resourcePath, aiMaterial* material, Mesh& fillMesh);
    void LoadTextureWithType_(const std::filesystem::path& resourcePath, aiMaterial* material, aiTextureType type, std::vector<std::reference_wrapper<Texture>>& meshTextures);
};

#endif // !_OPENGLFRAMEWORK_MODEL_H_
