#include "SpecialModel.h"

static const glm::vec3 c_cubeVertices[] = {
    { 0, 0, 0 },
    { 1, 1, 0 },
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 0 },
    { 0, 1, 1 },
    { 0, 1, 0 },
    { 0, 0, 1 },
    { 0, 1, 0 },
    { 1, 1, 1 },
    { 1, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 0 },
    { 1, 1, 0 },
    { 1, 1, 1 },
    { 1, 0, 1 },
    { 0, 0, 0 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 0, 0, 1 },
    { 0, 0, 1 },
    { 1, 0, 1 },
    { 1, 1, 1 },
    { 0, 1, 1 }
};

static const glm::ivec3 c_cubeTriangles[] = {
    { 0, 1, 2 },
    { 0, 3, 1 },
    { 4, 5, 6 },
    { 4, 7, 5 },
    { 8, 9, 10 },
    { 8, 11, 9 },
    { 12, 13, 14 },
    { 12, 14, 15 },
    { 16, 17, 18 },
    { 16, 18, 19 },
    { 20, 21, 22 },
    { 20, 22, 23 }
};

static const glm::vec3 c_cubeNormals[] = {
    { 0, 0, -1 },
    { 0, 0, -1 },
    { 0, 0, -1 },
    { 0, 0, -1 },
    { -1, 0, 0 },
    { -1, 0, 0 },
    { -1, 0, 0 },
    { -1, 0, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 0, 1, 0 },
    { 1, 0, 0 },
    { 1, 0, 0 },
    { 1, 0, 0 },
    { 1, 0, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 },
    { 0, -1, 0 },
    { 0, 0, 1 },
    { 0, 0, 1 },
    { 0, 0, 1 },
    { 0, 0, 1 } 
};

namespace OpenGLFramework::Core
{

template<>
BasicTriMesh Cube::GetBasicTriMesh()
{
    return BasicTriMesh{ 
        std::vector(std::cbegin(c_cubeVertices), std::cend(c_cubeVertices)),
        std::vector(std::cbegin(c_cubeTriangles), std::cend(c_cubeTriangles))
    };
};

template<>
BasicTriRenderMesh Cube::GetBasicTriRenderMesh()
{
    return BasicTriRenderMesh {
        GetBasicTriMesh(),
        std::vector(std::cbegin(c_cubeNormals), std::cend(c_cubeNormals))
    };
};

template<>
BasicTriModel Cube::GetBasicTriModel() {
    std::vector<BasicTriMesh> meshes;
    meshes.push_back(GetBasicTriMesh());

    return BasicTriModel{ std::move(meshes) };
};

template<>
BasicTriRenderModel Cube::GetBasicTriRenderModel(){
    std::vector<BasicTriRenderMesh> meshes;
    meshes.push_back(GetBasicTriRenderMesh());

    return BasicTriRenderModel{ std::move(meshes) };
};

}