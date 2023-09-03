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
    virtual ~IVertexAttibContainerFactory() = default;
};

template<std::derived_from<IVertexAttribContainer> VAC>
class NaiveVACFactory : public IVertexAttibContainerFactory { 
public:
    std::unique_ptr<IVertexAttribContainer> Create() override { 
        return std::make_unique<VAC>(); 
    } 
};

}

// NOTICE: It's more flexible for memory to define like this, but we don't 
// optimize that much because we focus on graphics algoriths mostly.
// 
//class IVertexAttibContainerFactory
//{
//public:
//    virtual void Destroy(IVertexAttribContainer* ptr) = 0;
//    class FactoryDeleter
//    {
//    public:
//        FactoryDeleter(IVertexAttibContainerFactory* init) : deleter{ init } {}
//        void operator()(IVertexAttribContainer* ptr) { deleter->Destroy(ptr); }
//    private:
//        IVertexAttibContainerFactory* deleter;
//    };
//
//    virtual std::unique_ptr<IVertexAttribContainer, FactoryDeleter> Create() = 0;
//    virtual ~IVertexAttibContainerFactory() = default;
//};
//
//using VACUniquePtr = std::unique_ptr<IVertexAttribContainer,
//    IVertexAttibContainerFactory::FactoryDeleter>;
//
//template<std::derived_from<IVertexAttribContainer> VAC>
//class NaiveVACFactory : public IVertexAttibContainerFactory {
//public:
//    virtual void Destroy(IVertexAttribContainer* ptr) { delete ptr; };
//    VACUniquePtr Create() override {
//        return std::unique_ptr<VAC, FactoryDeleter>{ new VAC{}, FactoryDeleter{ this }};
//    }
//};
