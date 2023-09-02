#pragma once
#include "TypeMapping.h"
#include <glad/glad.h>

#include <ranges>
#include <memory>

struct aiMesh;
#define BEGIN_BIND(type, posSize, vertexNum) { \
    using VertexAttrib = type; size_t initOffset = posSize * vertexNum;
#define BIND_VERTEX_ATTRIB(id, rawType, member) glEnableVertexAttribArray(id);\
        glVertexAttribPointer(\
            id, sizeof(VertexAttrib{}.member) / sizeof(rawType),\
            OpenGLFramework::GLHelper::ToGLType<rawType>::value, GL_FALSE,\
            sizeof(VertexAttrib), reinterpret_cast<void*>(\
                initOffset + offsetof(VertexAttrib, member)));
#define END_BIND }

namespace OpenGLFramework::GLHelper
{

class IVertexAttribContainer
{
public:
    virtual void CopyFromMesh(const aiMesh*) = 0;
    virtual void AllocateAndBind(size_t posSize, size_t vertexNum) = 0;
    virtual ~IVertexAttribContainer() = default;
protected:
    template<typename Container>
    requires std::ranges::contiguous_range<Container>
    static void AllocateAttributes_(const Container& container, size_t posSize, size_t vertexNum) {
        using VertexAttrib = typename Container::value_type;
        static_assert(std::is_default_constructible_v<VertexAttrib>,
            "Vertex attribute should be able to be default constructed.");
        static_assert(std::is_trivially_copyable_v<VertexAttrib>,
            "Vertex attribute should be bitwise-copiable.");

        size_t verticesPosSize = vertexNum * posSize;
        glBufferData(GL_ARRAY_BUFFER, verticesPosSize +
            vertexNum * sizeof(VertexAttrib), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, verticesPosSize,
            vertexNum * sizeof(VertexAttrib), std::ranges::data(container));
    };
};

class IVertexAttibContainerFactory
{
public:
    virtual std::unique_ptr<IVertexAttribContainer> Create() = 0;
    ~IVertexAttibContainerFactory() = default;
};

template<std::derived_from<IVertexAttribContainer> VAC>
class NaiveVACFactory : public IVertexAttibContainerFactory { 
public:
    std::unique_ptr<IVertexAttribContainer> Create() override { 
        return std::make_unique<VAC>(); 
    } 
};

}