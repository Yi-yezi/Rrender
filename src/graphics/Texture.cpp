#include "graphics/Texture.h"
#include <stb_image.h>
#include <stdexcept>
#include <iostream>

namespace graphics {

    Texture::Texture(const std::string& path, bool useSRGB) {
        LoadFromFile(path, useSRGB);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &m_ID);
    }

    Texture::Texture(Texture&& other) noexcept {
        m_ID = other.m_ID;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_Channels = other.m_Channels;
        other.m_ID = 0;
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            glDeleteTextures(1, &m_ID);

            m_ID = other.m_ID;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_Channels = other.m_Channels;
            other.m_ID = 0;
        }
        return *this;
    }

    void Texture::LoadFromFile(const std::string& path, bool useSRGB) {


        unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);
        if (!data)
            throw std::runtime_error("Failed to load texture: " + path);

        GLenum format, internalFormat;
        switch (m_Channels) {
            case 1: format = internalFormat = GL_RED; break;
            case 3:
                format = GL_RGB;
                internalFormat = useSRGB ? GL_SRGB : GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                internalFormat = useSRGB ? GL_SRGB_ALPHA : GL_RGBA;
                break;
            default:
                stbi_image_free(data);
                throw std::runtime_error("Unsupported texture format: " + path);
        }

        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 常用采样设置
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
    }

    void Texture::Bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_ID);
    }

}
