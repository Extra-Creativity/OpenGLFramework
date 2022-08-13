#pragma once
#ifndef _OPENGLFRAMEWORK_TEXTURE_H_
#define _OPENGLFRAMEWORK_TEXTURE_H_
#include <string>
#include <filesystem>

class Texture
{
public:
    unsigned int ID;
    Texture(const std::filesystem::path& path);
    ~Texture();
    Texture(Texture&) = delete;
    Texture(Texture&& another) noexcept : ID(another.ID) { another.ID = 0; };
};

#endif // !_OPENGLFRAMEWORK_TEXTURE_H_
