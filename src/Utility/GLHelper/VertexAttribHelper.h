#pragma once
#include "TypeMapping.h"
#include <glad/glad.h>

#include <any>
#include <ranges>
#include <utility>

struct aiMesh;
namespace OpenGLFramework::GLHelper
{
template<typename T>
struct VertexAttribHelper { };

template<typename T>
struct VATag {
    static const bool flag = false;
};

template<typename Container>
concept VertexAttribContainer = requires() {
    typename Container::value_type;
    requires std::ranges::contiguous_range<Container>
        && VATag<typename Container::value_type>::flag;
};

template<typename T>
void CopyVertexAttributes(T&, const aiMesh*);

class IVertexAttribContainer {
private:
    std::any container_ = {};
    void(*allocAndBind_)(std::any&, size_t, size_t) = nullptr;
    void(*copyFromMesh_)(std::any&, const aiMesh*) = nullptr;

public:
    IVertexAttribContainer() = default;

    template<typename Container>
    requires VertexAttribContainer<Container>
        IVertexAttribContainer(Container obj) :
        container_{ std::move(obj) }
    {
        using VertexAttrib = typename Container::value_type;
        allocAndBind_ = [](std::any& obj, size_t posSize, size_t vertexNum) {
            size_t verticesPosSize = vertexNum * posSize;
            glBufferData(GL_ARRAY_BUFFER, verticesPosSize +
                vertexNum * sizeof(VertexAttrib), nullptr, GL_STATIC_DRAW);
            auto& container = std::any_cast<Container&>(obj);
            glBufferSubData(GL_ARRAY_BUFFER, verticesPosSize,
                vertexNum * sizeof(VertexAttrib), std::ranges::data(container));

            VertexAttribHelper<VertexAttrib>::Bind(verticesPosSize);
        };
        copyFromMesh_ = [](std::any& obj, const aiMesh* mesh) {
            CopyVertexAttributes(std::any_cast<Container&>(obj), mesh);
        };
    }

    void AllocateAndBind(size_t singleSize, size_t vertexNum)
    {
        allocAndBind_(container_, singleSize, vertexNum);
    }

    void CopyFromMesh(const aiMesh* mesh){ copyFromMesh_(container_, mesh); }
};
}

#define BEGIN_REFLECT(type)\
static_assert(std::is_default_constructible_v<type>, \
             "Vertex attribute should be able to be default constructed.");\
static_assert(std::is_trivially_copyable_v<type>,\
             "Vertex attribute should be bitwise-copiable.");\
template<> struct OpenGLFramework::GLHelper::VATag<type> {\
    static const bool flag = true; \
}; \
template<> struct OpenGLFramework::GLHelper::VertexAttribHelper<type> {\
    using VertexAttrib = type;\
    template<size_t id, typename=void>\
    struct Reflect { \
        static inline void Bind(size_t off){ return; }\
    };\
    template<size_t... Indices>\
    static inline void InnerBind(std::index_sequence<Indices...>, size_t off)\
    { (Reflect<Indices + 1>::Bind(off), ...); }

#define REFLECT(id, rawType, member)\
template<typename T>\
struct Reflect<id, T>{\
    using RawType = rawType;\
    using Type = decltype(VertexAttrib{}.member);\
    static inline void Bind(size_t initOffset){\
        glEnableVertexAttribArray(id);\
        glVertexAttribPointer(\
            id, sizeof(Type) / sizeof(RawType), ToGLType<rawType>::value,\
            GL_FALSE, sizeof(VertexAttrib), reinterpret_cast<void*>(\
                initOffset + offsetof(VertexAttrib, member)));\
    }\
};

#define END_REFLECT(total) static inline void Bind(size_t off)\
    { InnerBind(std::make_index_sequence<total>{}, off); };\
    };
// end of VertexAttribHelper.

#define VERTEX_ATTRIB_SPECIALIZE_COPY template<>\
inline void OpenGLFramework::GLHelper::CopyVertexAttributes
