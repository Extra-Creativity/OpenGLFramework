#include "SpecialModel.h"

static const glm::vec3 c_cubeVertices[] = {
    { 0.0 , 0.0 , 0.0 },
    { 0.0 , 0.0 , 1.0 },
    { 0.0 , 1.0 , 0.0 },
    { 0.0 , 1.0 , 1.0 },
    { 1.0 , 0.0 , 0.0 },
    { 1.0 , 0.0 , 1.0 },
    { 1.0 , 1.0 , 0.0 },
    { 1.0 , 1.0 , 1.0 }
};

static const glm::ivec3 c_cubeTriangles[] = {
    { 0, 6, 4 },
    { 0, 2, 6 },
    { 0, 3, 2 },
    { 0, 1, 3 },
    { 2, 7, 6 },
    { 2, 3, 7 },
    { 4, 6, 7 },
    { 4, 7, 5 },
    { 0, 4, 5 },
    { 0, 5, 1 },
    { 1, 5, 7 },
    { 1, 7, 3 }
};

static const glm::vec3 c_cubeNormals[] = {
    { 0.0, 0.0, 1.0 },
    { 0.0, 0.0, -1.0 },
    { 0.0, 1.0, 0.0 },
    { 0.0, -1.0, 0.0 },
    { 1.0, 0.0, 0.0 },
    { -1.0, 0.0, 0.0 }
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