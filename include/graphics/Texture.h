#pragma once

#include <string>
#include <glad/glad.h>
#include <vector>

namespace graphics {
    
    enum TextureSlots {
        // glTF 2.0 标准纹理槽位 (0-6)
        TEX_SLOT_BASE_COLOR      = 0,  // 修正：TEX_SLOT_ALBEDO -> TEX_SLOT_BASE_COLOR
        TEX_SLOT_NORMAL          = 1,
        TEX_SLOT_METALLIC_ROUGHNESS = 2,  // 修正：分离的 metallic/roughness -> 合并的 metallic-roughness
        TEX_SLOT_OCCLUSION       = 3,  // 修正：TEX_SLOT_AO -> TEX_SLOT_OCCLUSION
        TEX_SLOT_EMISSION        = 4,
        TEX_SLOT_SPECULAR_GLOSSINESS = 5,  // glTF 扩展：Specular-Glossiness 工作流
        TEX_SLOT_DIFFUSE         = 6,  // glTF 扩展：Specular-Glossiness 工作流的 diffuse
        
        // G-Buffer slots (7-9)
        TEX_SLOT_GBUFFER_ALBEDO   = 7, // GL_TEXTURE_2D
        TEX_SLOT_GBUFFER_NORMAL   = 8, // GL_TEXTURE_2D
        TEX_SLOT_GBUFFER_POSITION = 9, // GL_TEXTURE_2D
        
        // 阴影贴图（全部用array，shader用layer区分）(10-12)
        TEX_SLOT_SHADOW_DIR  = 10, // GL_TEXTURE_2D_ARRAY，方向光
        TEX_SLOT_SHADOW_POINT= 11, // GL_TEXTURE_CUBE_MAP_ARRAY，点光源
        TEX_SLOT_SHADOW_SPOT = 12, // GL_TEXTURE_2D_ARRAY，聚光灯

        // IBL (13-16)
        TEX_SLOT_IRRADIANCE  = 13,
        TEX_SLOT_PREFILTER   = 14,
        TEX_SLOT_BRDF_LUT    = 15,
        TEX_SLOT_ENV_CUBEMAP = 16, // 环境立方体贴图
        
        // 向后兼容的别名 (deprecated - 应该逐渐移除)
        TEX_SLOT_ALBEDO      = TEX_SLOT_BASE_COLOR,  // 别名，向后兼容
        TEX_SLOT_METALLIC    = TEX_SLOT_METALLIC_ROUGHNESS,  // 别名，向后兼容
        TEX_SLOT_ROUGHNESS   = TEX_SLOT_METALLIC_ROUGHNESS,  // 别名，向后兼容
        TEX_SLOT_AO          = TEX_SLOT_OCCLUSION,   // 别名，向后兼容
        TEX_SLOT_SPECULAR    = TEX_SLOT_SPECULAR_GLOSSINESS  // 别名，向后兼容
    };

    /**
     * @brief 封装 OpenGL 2D 纹理，RAII 管理，支持 sRGB 格式
     */
    class Texture {
    public:
        /**
         * @brief 构造并加载纹理
         * @param path      纹理图片路径
         * @param useSRGB   是否使用sRGB格式加载（用于PBR或Gamma矫正）
         * @param useFloat  是否使用浮点格式 (HDR纹理)
         */
        explicit Texture(const std::string &path, bool useSRGB = false, bool useFloat = false);
        
        /**
         * @brief 从内存数据构造纹理
         * @param imageData 图像数据
         * @param width     图像宽度
         * @param height    图像高度
         * @param useSRGB   是否使用sRGB格式
         * @param useFloat  是否使用浮点格式
         */
        explicit Texture(const std::vector<unsigned char>& imageData, int width, int height, 
                         bool useSRGB = false, bool useFloat = false);

        // 禁用拷贝构造和赋值
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // 支持移动构造与赋值
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        /// 析构函数：自动释放OpenGL资源
        ~Texture();

        /// 绑定纹理到指定纹理单元
        void Bind(int slot) const;    // 绑定到指定槽位
        void Bind() const;            // 绑定到默认槽位 0

        /// 获取OpenGL纹理ID
        unsigned int GetID() const { return m_ID; }

        /// 获取纹理尺寸
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        int GetChannels() const { return m_Channels; }

        /// 获取纹理格式信息
        bool IsSRGB() const { return m_UseSRGB; }
        bool IsFloat() const { return m_UseFloat; }

    private:
        unsigned int m_ID = 0;
        int m_Width = 0, m_Height = 0, m_Channels = 0;
        bool m_UseSRGB = false;   // 是否使用 sRGB 格式
        bool m_UseFloat = false;  // 是否使用浮点格式

        void LoadFromFile(const std::string& path, bool useSRGB, bool useFloat = false);
    };

} // namespace graphics
