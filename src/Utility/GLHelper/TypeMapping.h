#pragma once
#include <glad/glad.h>

template<typename T>
struct ToGLType {};

template<>
struct ToGLType<float> {
    static const auto value = GL_FLOAT;
};

template<>
struct ToGLType<int> {
    static const auto value = GL_INT;
};
