#pragma once

#include <string>
#include <glad/glad.h>

namespace graphics {

    /**
     * @brief 封装 OpenGL 2D 纹理，RAII 管理，支持 sRGB 格式
     */
    class Texture {
    public:
        /**
         * @brief 构造并加载纹理
         * @param path      纹理图片路径
         * @param useSRGB   是否使用sRGB格式加载（用于PBR或Gamma矫正）
         */
        explicit Texture(const std::string& path, bool useSRGB = false);

        // 禁用拷贝构造和赋值
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // 支持移动构造与赋值
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        /// 析构函数：自动释放OpenGL资源
        ~Texture();

        /// 绑定纹理到指定纹理单元
        void Bind(unsigned int slot = 0) const;

        /// 获取OpenGL纹理ID
        unsigned int GetID() const { return m_ID; }

        /// 获取纹理尺寸
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

    private:
        unsigned int m_ID = 0;
        int m_Width = 0, m_Height = 0, m_Channels = 0;

        void LoadFromFile(const std::string& path, bool useSRGB);
    };

} // namespace graphics
