#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "graphics/Material.h"

namespace graphics {
    class Shader;

    /**
     * @brief 组合多个Mesh和Material，实现模型的加载与绘制
     */
    class Model {
    public:
        /**
         * @brief 加载模型
         * @param path    模型文件路径（支持 .obj, .gltf, .glb）
         * @param useSRGB 是否以sRGB格式加载纹理
         * @param drawMode 绘制模式（如GL_TRIANGLES/GL_TRIANGLE_STRIP）
         */
        Model(const std::string& path, bool useSRGB = true, GLenum drawMode = GL_TRIANGLES);
        
        /**
         * @brief 从顶点数据创建单纹理模型
         */
        Model(const std::vector<Vertex>& vertices, const std::string& texturePath, bool useSRGB, GLenum drawMode = GL_TRIANGLES);
        
        /**
         * @brief 从顶点数据创建多纹理模型
         */
        Model(const std::vector<Vertex>& vertices, const std::vector<std::string>& texturePaths, bool useSRGB, GLenum drawMode = GL_TRIANGLES);
        
        /**
         * @brief 从顶点和索引数据创建多纹理模型
         */
        Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<std::string>& texturePaths, bool useSRGB, GLenum drawMode = GL_TRIANGLES);
        
        /**
         * @brief 从顶点和索引数据创建单纹理模型
         */
        Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::string& texturePath, bool useSRGB, GLenum drawMode = GL_TRIANGLES);
        
        ~Model() = default;

        // 禁用拷贝构造和赋值
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        // 支持移动构造和赋值
        Model(Model&&) noexcept = default;
        Model& operator=(Model&&) noexcept = default;

        /**
         * @brief 绘制模型（推荐方式：使用 Material 系统）
         * @param shader 用于渲染的着色器程序
         */
        void Draw(std::shared_ptr<Shader> shader) const;


        /**
         * @brief 获取子网格数量
         */
        size_t GetMeshCount() const { return m_Meshes.size(); }

    private:
        /**
         * @brief 网格与材质的组合
         */
        struct TexturedMesh {
            Mesh mesh;
            std::shared_ptr<Material> material;  // 修正：使用 Material 而不是纹理列表
        };

        std::vector<TexturedMesh> m_Meshes; ///< 所有子网格及其材质
        std::string m_Directory; ///< 模型文件所在目录
        

        GLenum m_DrawMode = GL_TRIANGLES; // 绘制模式

        /**
         * @brief 根据文件扩展名选择加载器
         */
        void LoadModel(const std::string& path, bool useSRGB);
        
        /**
         * @brief 加载 OBJ 模型
         */
        void LoadObjModel(const std::string& path, bool useSRGB);
        
        /**
         * @brief 加载 glTF/GLB 模型
         */
        void LoadGltfModel(const std::string& path, bool useSRGB);

        /**
         * @brief 处理tinyobj的shape和材质，按材质分组生成Mesh
         */
        void ProcessMeshData(const void* attrib,
                             const void* shape,
                             const void* materials,
                             size_t materialCount,
                             bool useSRGB);

        /**
         * @brief 加载材质纹理，带缓存（修正：移除 TextureType 参数）
         * @param baseDir 基础目录
         * @param texPath 纹理文件路径
         * @param useSRGB 是否使用 sRGB 格式
         * @return 纹理对象指针，失败时返回 nullptr
         */
        std::shared_ptr<Texture> LoadMaterialTexture(const std::string& baseDir,
                                                     const std::string& texPath,
                                                     bool useSRGB);
    };

} // namespace graphics
