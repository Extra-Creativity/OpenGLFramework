#include "Texture.h"
#include "Utility/IO/IOExtension.h"
#include <version>
#ifdef __cpp_lib_format	
#include <format>
#endif
#include <glad/glad.h>
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

const int fileMaxLen = 1024;

namespace OpenGLFramework::Core
{
Texture::Texture(const std::filesystem::path& path) : ID(0)
{
    glGenTextures(1, &ID);
    int width = 0, height = 0, channels = 0;
#ifdef _WIN32
    std::string stbiPath(fileMaxLen, '\0');
    stbi_convert_wchar_to_utf8(const_cast<char*>(stbiPath.c_str()), fileMaxLen, path.c_str());
    const char* validPath = stbiPath.c_str();
#else
     const char* validPath = path.c_str();
#endif // _WIN32

    unsigned char* imageData = stbi_load(validPath, &width, &height, &channels, 0);
    if (imageData == nullptr) [[unlikely]]
    {
#   ifdef __cpp_lib_format	
        IOExtension::LogError(std::format("Fail to load texture image at "
            "path {} for reason : {}", path.string(), stbi_failure_reason()));
#   else
        IOExtension::LogError("Fail to load texture image at path " 
            + path.string() + " for reason : " + stbi_failure_reason());
#   endif
        stbi_image_free(imageData);
        return;
    }
    GLenum format;
    switch (channels)
    {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    default: [[unlikely]]
#   ifdef __cpp_lib_format	
        IOExtension::LogError(
            std::format("Unknown image channel type : {}", channels));
#   else
        IOExtension::LogError(
            "Unknown image channel type : " + std::to_string(channels));
#   endif
        format = GL_RGBA;
        break;
    }

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(imageData);
    return;
};

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}

} // namespace OpenGLFramework::Core
