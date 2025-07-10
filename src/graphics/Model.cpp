#include "graphics/Model.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace graphics {

    // 构造函数：加载模型
    Model::Model(const std::string& path, bool useSRGB) {
        LoadModel(path, useSRGB);
    }

    // 绘制所有子网格及其纹理
    void Model::Draw() const {
        for (const auto& texturedMesh : m_Meshes) {
            // 绑定所有纹理到不同的纹理单元
            for (size_t i = 0; i < texturedMesh.textures.size(); ++i) {
                texturedMesh.textures[i]->Bind(static_cast<unsigned int>(i));
            }
            // 绘制网格
            texturedMesh.mesh.Draw();
        }
    }

    // 加载OBJ模型，解析shapes和materials
    void Model::LoadModel(const std::string& path, bool useSRGB) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        // 获取模型文件所在目录
        std::string baseDir = std::filesystem::path(path).parent_path().string();
        if (!baseDir.empty() && baseDir.back() != '/' && baseDir.back() != '\\')
            baseDir += '/';

        // 加载OBJ文件
        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                        path.c_str(), baseDir.c_str());

        if (!warn.empty()) std::cout << "[ModelLoader Warning] " << warn << std::endl;
        if (!err.empty()) std::cerr << "[ModelLoader Error] " << err << std::endl;
        if (!success) throw std::runtime_error("Failed to load model: " + path);

        m_Directory = baseDir;

        // 处理每个shape
        for (const auto& shape : shapes) {
            ProcessMeshData(&attrib, &shape, materials.data(), materials.size(), useSRGB);
        }
    }

    // 处理单个shape，按材质分组生成Mesh
    void Model::ProcessMeshData(const void* attribPtr,
                                const void* shapePtr,
                                const void* materialsPtr,
                                size_t materialCount,
                                bool useSRGB) {
        const tinyobj::attrib_t* attrib = static_cast<const tinyobj::attrib_t*>(attribPtr);
        const tinyobj::shape_t* shape = static_cast<const tinyobj::shape_t*>(shapePtr);
        const tinyobj::material_t* materials = static_cast<const tinyobj::material_t*>(materialsPtr);

        // 1. 按材质分组，每组独立顶点和索引
        std::map<int, std::vector<Vertex>> matID_to_vertices;
        std::map<int, std::vector<unsigned int>> matID_to_indices;
        std::map<int, std::map<std::tuple<int, int, int>, unsigned int>> matID_to_vertexCache;

        size_t index_offset = 0;
        for (size_t f = 0; f < shape->mesh.num_face_vertices.size(); ++f) {
            int fv = shape->mesh.num_face_vertices[f];
            int matID = shape->mesh.material_ids.empty() ? -1 : shape->mesh.material_ids[f];

            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape->mesh.indices[index_offset + v];
                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                unsigned int vertIndex;
                auto& vertexCache = matID_to_vertexCache[matID];
                auto& vertices = matID_to_vertices[matID];
                // 顶点去重：属性三元组唯一
                if (vertexCache.find(key) != vertexCache.end()) {
                    vertIndex = vertexCache[key];
                } else {
                    Vertex vertex{};
                    vertex.Position = {
                        attrib->vertices[3 * idx.vertex_index + 0],
                        attrib->vertices[3 * idx.vertex_index + 1],
                        attrib->vertices[3 * idx.vertex_index + 2]
                    };
                    if (idx.normal_index >= 0) {
                        vertex.Normal = {
                            attrib->normals[3 * idx.normal_index + 0],
                            attrib->normals[3 * idx.normal_index + 1],
                            attrib->normals[3 * idx.normal_index + 2]
                        };
                    }
                    if (idx.texcoord_index >= 0) {
                        vertex.TexCoords = {
                            attrib->texcoords[2 * idx.texcoord_index + 0],
                            attrib->texcoords[2 * idx.texcoord_index + 1]
                        };
                    }
                    vertIndex = static_cast<unsigned int>(vertices.size());
                    vertices.push_back(vertex);
                    vertexCache[key] = vertIndex;
                }
                matID_to_indices[matID].push_back(vertIndex);
            }
            index_offset += fv;
        }

        // 2. 为每种材质生成 mesh 和纹理
        for (const auto& [matID, indices] : matID_to_indices) {
            std::vector<std::shared_ptr<Texture>> textures;
            if (matID >= 0 && matID < static_cast<int>(materialCount)) {
                const auto& mat = materials[matID];
                // 加载漫反射纹理
                if (!mat.diffuse_texname.empty()) {
                    if (auto tex = LoadMaterialTexture(m_Directory, mat.diffuse_texname, useSRGB)) {
                        textures.push_back(tex);
                    }
                }
                // 加载高光纹理
                if (!mat.specular_texname.empty()) {
                    if (auto tex = LoadMaterialTexture(m_Directory, mat.specular_texname, useSRGB)) {
                        textures.push_back(tex);
                    }
                }
                // 加载法线/凹凸纹理
                if (!mat.bump_texname.empty()) {
                    if (auto tex = LoadMaterialTexture(m_Directory, mat.bump_texname, useSRGB)) {
                        textures.push_back(tex);
                    }
                }
            }
            // 生成子网格
            m_Meshes.emplace_back(TexturedMesh{Mesh(matID_to_vertices[matID], indices), textures});
        }
    }

    // 路径到纹理的缓存，避免重复加载
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureCache;

    // 加载材质纹理，带缓存
    std::shared_ptr<Texture> Model::LoadMaterialTexture(const std::string& baseDir,
                                                        const std::string& texPath,
                                                        bool useSRGB) {
        std::filesystem::path fullPath = std::filesystem::path(baseDir) / texPath;
        std::string fullPathStr = fullPath.string();

        auto it = m_TextureCache.find(fullPathStr);
        if (it != m_TextureCache.end()) {
            return it->second;
        }

        try {
            auto tex = std::make_shared<Texture>(fullPathStr, useSRGB);
            m_TextureCache[fullPathStr] = tex;
            return tex;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load texture: " << fullPathStr << "\nReason: " << e.what() << std::endl;
            return nullptr;
        }
    }

} // namespace graphics
