#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "graphics/Mesh.h"
#include "graphics/Texture.h"


namespace graphics {

    /**
     * @brief 组合多个Mesh和纹理，实现OBJ模型的加载与绘制
     */
    class Model {
    public:
        /**
         * @brief 加载模型
         * @param path    OBJ文件路径
         * @param useSRGB 是否以sRGB格式加载纹理
         */
        Model(const std::string& path, bool useSRGB = true);
        ~Model() = default;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        Model(Model&&) noexcept = default;
        Model& operator=(Model&&) noexcept = default;

        /**
         * @brief 绘制模型（自动绑定所有纹理）
         */
        void Draw() const;

    private:
        struct TexturedMesh {
            Mesh mesh;
            std::vector<std::shared_ptr<Texture>> textures;
        };

        std::vector<TexturedMesh> m_Meshes; ///< 所有子网格及其纹理
        std::string m_Directory; ///< 模型文件所在目录

        // 路径到纹理的缓存，避免重复加载
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureCache;

        /**
         * @brief 加载OBJ模型
         */
        void LoadModel(const std::string& path, bool useSRGB);

        /**
         * @brief 处理tinyobj的shape和材质，按材质分组生成Mesh
         */
        void ProcessMeshData(const void* attrib,
                             const void* shape,
                             const void* materials,
                             size_t materialCount,
                             bool useSRGB);

        /**
         * @brief 加载材质纹理，带缓存
         */
        std::shared_ptr<Texture> LoadMaterialTexture(const std::string& baseDir,
                                                     const std::string& texPath,
                                                     bool useSRGB);
    };

} // namespace graphics
