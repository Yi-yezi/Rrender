#include "graphics/Texture.h"
#include <stb_image.h>
#include <stdexcept>
#include <iostream>

namespace graphics {

    Texture::Texture(const std::string& path, bool useSRGB, bool useFloat) {
        LoadFromFile(path, useSRGB, useFloat);
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

    void Texture::LoadFromFile(const std::string& path, bool useSRGB, bool useFloat) {
        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);

        int width, height, channels;
        void* data = nullptr;

        GLenum internalFormat = GL_RGB8;
        GLenum dataFormat = GL_RGB;
        GLenum type = GL_UNSIGNED_BYTE;

        if (useFloat) {
            // 加载HDR/浮点纹理
            float* fdata = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
            if (!fdata) {
                throw std::runtime_error("Failed to load HDR texture: " + path);
            }
            data = fdata;
            type = GL_FLOAT;
            // 这里优先用16F，节省显存且大多数场景足够用
            if (channels == 1) {
                internalFormat = GL_R16F;
                dataFormat = GL_RED;
            } else if (channels == 3) {
                internalFormat = GL_RGB16F;
                dataFormat = GL_RGB;
            } else if (channels == 4) {
                internalFormat = GL_RGBA16F;
                dataFormat = GL_RGBA;
            }
        } else {
            // 加载普通8位纹理
            unsigned char* cdata = stbi_load(path.c_str(), &width, &height, &channels, 0);
            if (!cdata) {
                throw std::runtime_error("Failed to load texture: " + path);
            }
            data = cdata;
            if (channels == 1) {
                internalFormat = GL_R8;
                dataFormat = GL_RED;
            } else if (channels == 3) {
                internalFormat = useSRGB ? GL_SRGB8 : GL_RGB8;
                dataFormat = GL_RGB;
            } else if (channels == 4) {
                internalFormat = useSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                dataFormat = GL_RGBA;
            }
        }

        m_Width = width;
        m_Height = height;
        m_Channels = channels;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, type, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 释放stb_image数据
        stbi_image_free(data);
    }

    void Texture::Bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_ID);
    }

}
