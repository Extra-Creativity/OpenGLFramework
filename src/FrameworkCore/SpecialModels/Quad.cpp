#include "SpecialModel.h"

static const glm::vec3 c_quadVertices[] = {
    { -1.0 , 1.0 , 0.0 },
    { 1.0 , 1.0 , 0.0 },
    { 1.0 , -1.0 , 0.0 },
    { -1.0 , -1.0 , 0.0 }
};

static const glm::ivec3 c_quadTriangles[] = {
    { 0, 1, 2 },
    { 0, 2, 3 }
};

static const OpenGLFramework::Core::BasicVertexAttribute c_quadAttributes[] = {
    { { 0.0, 0.0, 1.0 }, { 0.0, 1.0 } },
    { { 0.0, 0.0, 1.0 }, { 1.0, 1.0 } },
    { { 0.0, 0.0, 1.0 }, { 1.0, 0.0 } },
    { { 0.0, 0.0, 1.0 }, { 0.0, 0.0 } }
};

namespace OpenGLFramework::Core
{

template<>
BasicTriMesh Quad::GetBasicTriMesh()
{
    return BasicTriMesh{
        std::vector(std::cbegin(c_quadVertices), std::cend(c_quadVertices)),
        std::vector(std::cbegin(c_quadTriangles), std::cend(c_quadTriangles))
    };
};

template<>
BasicTriRenderMesh Quad::GetBasicTriRenderMesh()
{
    return BasicTriRenderMesh{
        GetBasicTriMesh(),
        std::vector(std::cbegin(c_quadAttributes), std::cend(c_quadAttributes))
    };
};

template<>
BasicTriModel Quad::GetBasicTriModel() {
    std::vector<BasicTriMesh> meshes;
    meshes.push_back(GetBasicTriMesh());

    return BasicTriModel{ std::move(meshes) };
};

template<>
BasicTriRenderModel Quad::GetBasicTriRenderModel() {
    std::vector<BasicTriRenderMesh> meshes;
    meshes.push_back(GetBasicTriRenderMesh());

    return BasicTriRenderModel{ std::move(meshes) };
};

}