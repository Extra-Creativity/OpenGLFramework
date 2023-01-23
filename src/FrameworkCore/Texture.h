#pragma once
#ifndef OPENGLFRAMEWORK_CORE_TEXTURE_H_
#define OPENGLFRAMEWORK_CORE_TEXTURE_H_
#include <string>
#include <filesystem>

namespace OpenGLFramework::Core
{

class Texture
{
public:
    unsigned int ID;
    Texture(const std::filesystem::path& path);
    ~Texture();
    Texture(Texture&) = delete;
    Texture(Texture&& another) noexcept : ID(another.ID) { another.ID = 0; };
};

} // namespace OpenGLFramework::Core
#endif // !OPENGLFRAMEWORK_CORE_TEXTURE_H_
