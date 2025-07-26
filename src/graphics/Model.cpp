#include "graphics/Model.h"
#include "graphics/Material.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cstdio>


    

namespace graphics {
    // 构造函数：加载模型
    Model::Model(const std::string& path, bool useSRGB, GLenum drawMode)
    {
        m_DrawMode = drawMode;
        LoadModel(path, useSRGB);
    }

    // 简化构造函数：创建单纹理材质
    Model::Model(const std::vector<Vertex>& vertices, const std::string& texturePath, bool useSRGB, GLenum drawMode)
    {
        auto material = std::make_shared<Material>();
        if (!texturePath.empty()) {
            if (auto tex = LoadMaterialTexture("", texturePath, useSRGB)) {
                material->SetBaseColorTexture(tex);  // 假设是基础颜色纹理
                material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
            } else {
                material->workflow = MaterialWorkflow::NO_TEXTURE;
            }
        } else {
            material->workflow = MaterialWorkflow::NO_TEXTURE;
        }
        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, drawMode), material});
    }

    // 多纹理构造函数：创建多纹理材质
    Model::Model(const std::vector<Vertex>& vertices, const std::vector<std::string>& texturePaths, bool useSRGB, GLenum drawMode)
    {
        auto material = std::make_shared<Material>();
        material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
        
        // 简单分配：第一个纹理作为基础颜色，其他按顺序分配
        for (size_t i = 0; i < texturePaths.size() && i < 7; ++i) {
            if (auto tex = LoadMaterialTexture("", texturePaths[i], useSRGB)) {
                switch (i) {
                    case 0: material->SetBaseColorTexture(tex); break;
                    case 1: material->SetNormalTexture(tex); break;
                    case 2: material->SetMetallicRoughnessTexture(tex); break;
                    case 3: material->SetOcclusionTexture(tex); break;
                    case 4: material->SetEmissionTexture(tex); break;
                    default: break;
                }
            }
        }
        
        if (!material->HasAnyTexture()) {
            material->workflow = MaterialWorkflow::NO_TEXTURE;
        }
        
        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, drawMode), material});
    }

    // 带索引的构造函数
    Model::Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::string& texturePath, bool useSRGB, GLenum drawMode)
    {
        auto material = std::make_shared<Material>();
        if (!texturePath.empty()) {
            if (auto tex = LoadMaterialTexture("", texturePath, useSRGB)) {
                material->SetBaseColorTexture(tex);
                material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
            } else {
                material->workflow = MaterialWorkflow::NO_TEXTURE;
            }
        } else {
            material->workflow = MaterialWorkflow::NO_TEXTURE;
        }
        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, indices, drawMode), material});
    }

    Model::Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<std::string>& texturePaths, bool useSRGB, GLenum drawMode)
    {
        auto material = std::make_shared<Material>();
        material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
        
        // 简单分配纹理
        for (size_t i = 0; i < texturePaths.size() && i < 7; ++i) {
            if (auto tex = LoadMaterialTexture("", texturePaths[i], useSRGB)) {
                switch (i) {
                    case 0: material->SetBaseColorTexture(tex); break;
                    case 1: material->SetNormalTexture(tex); break;
                    case 2: material->SetMetallicRoughnessTexture(tex); break;
                    case 3: material->SetOcclusionTexture(tex); break;
                    case 4: material->SetEmissionTexture(tex); break;
                    default: break;
                }
            }
        }
        
        if (!material->HasAnyTexture()) {
            material->workflow = MaterialWorkflow::NO_TEXTURE;
        }
        
        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, indices, drawMode), material});
    }

    // 绘制所有子网格及其材质
    void Model::Draw(std::shared_ptr<Shader> shader) const {
        for (const auto& texturedMesh : m_Meshes) {
            // 使用材质设置 uniform 和绑定纹理
            if (texturedMesh.material) {
                texturedMesh.material->SetUniforms(shader);
                texturedMesh.material->BindTextures(shader);
            }
            
            // 绘制网格
            texturedMesh.mesh.Draw();
        }
    }

    void Model::LoadModel(const std::string& path, bool useSRGB) {
        auto ext = std::filesystem::path(path).extension().string();
        if (ext == ".obj") {
            LoadObjModel(path, useSRGB);
        } else if (ext == ".gltf" || ext == ".glb") {
            LoadGltfModel(path, useSRGB);
        } else {
            throw std::runtime_error("Unsupported model format: " + ext);
        }
    }

    void Model::LoadGltfModel(const std::string& path, bool useSRGB) {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF loader;
        std::string err, warn;
        
        bool isBinary = (path.substr(path.find_last_of('.')) == ".glb");
        bool success = isBinary ?
            loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path) :
            loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
        
        if (!success) throw std::runtime_error("Failed to load glTF model: " + path);
        
        m_Directory = std::filesystem::path(path).parent_path().string() + "/";
        
        for (const auto& mesh : gltfModel.meshes) {
            for (const auto& primitive : mesh.primitives) {
                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                
                // 处理顶点数据（保持不变）
                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const auto& accessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    const float* posData = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; ++i) {
                        Vertex vertex{};
                        vertex.Position = glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
                        vertices.push_back(vertex);
                    }
                }
                
                // 处理法线
                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const auto& accessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    const float* normData = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; ++i) {
                        vertices[i].Normal = glm::vec3(normData[i * 3], normData[i * 3 + 1], normData[i * 3 + 2]);
                    }
                }
                
                // 处理纹理坐标
                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const auto& accessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    const float* texData = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; ++i) {
                        vertices[i].TexCoords = glm::vec2(texData[i * 2], texData[i * 2 + 1]);
                    }
                }
                
                // 处理索引
                if (primitive.indices >= 0) {
                    const auto& accessor = gltfModel.accessors[primitive.indices];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];
                    
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const unsigned int* indexData = reinterpret_cast<const unsigned int*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            indices.push_back(indexData[i]);
                        }
                    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const unsigned short* indexData = reinterpret_cast<const unsigned short*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            indices.push_back(static_cast<unsigned int>(indexData[i]));
                        }
                    }
                }
                
                // 创建材质对象
                auto material = std::make_shared<Material>();
                
                if (primitive.material >= 0) {
                    const auto& gltfMaterial = gltfModel.materials[primitive.material];
                    
                    // 设置 Alpha 模式
                    material->alphaMode = gltfMaterial.alphaMode;
                    material->alphaCutoff = gltfMaterial.alphaCutoff;
                    
                    // 检查 Specular-Glossiness 扩展
                    bool hasSpecularGlossiness = false;
                    if (!gltfMaterial.extensions.empty()) {
                        for (const auto& ext : gltfMaterial.extensions) {
                            if (ext.first == "KHR_materials_pbrSpecularGlossiness") {
                                material->workflow = MaterialWorkflow::SPECULAR_GLOSSINESS;
                                hasSpecularGlossiness = true;
                                
                                const auto& pbrSG = ext.second;
                                
                                // 提取因子
                                if (pbrSG.Has("diffuseFactor")) {
                                    auto df = pbrSG.Get("diffuseFactor");
                                    if (df.IsArray() && df.ArrayLen() >= 3) {
                                        material->diffuseColor = glm::vec3(
                                            df.Get(0).Get<double>(),
                                            df.Get(1).Get<double>(),
                                            df.Get(2).Get<double>()
                                        );
                                    }
                                }
                                
                                if (pbrSG.Has("specularFactor")) {
                                    auto sf = pbrSG.Get("specularFactor");
                                    if (sf.IsArray() && sf.ArrayLen() >= 3) {
                                        material->specularColor = glm::vec3(
                                            sf.Get(0).Get<double>(),
                                            sf.Get(1).Get<double>(),
                                            sf.Get(2).Get<double>()
                                        );
                                    }
                                }
                                
                                if (pbrSG.Has("glossinessFactor")) {
                                    material->glossinessFactor = pbrSG.Get("glossinessFactor").Get<double>();
                                }
                                
                                // 加载纹理
                                if (pbrSG.Has("diffuseTexture") && pbrSG.Get("diffuseTexture").Has("index")) {
                                    int diffuseTexIndex = pbrSG.Get("diffuseTexture").Get("index").Get<int>();
                                    if (diffuseTexIndex >= 0 && diffuseTexIndex < gltfModel.textures.size()) {
                                        const auto& texture = gltfModel.textures[diffuseTexIndex];
                                        const auto& image = gltfModel.images[texture.source];
                                        if (!image.image.empty()) {
                                            try {
                                                auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                                material->SetDiffuseTexture(tex);
                                            } catch (const std::exception& e) {
                                                // 静默处理异常
                                            }
                                        }
                                    }
                                }
                                
                                if (pbrSG.Has("specularGlossinessTexture") && pbrSG.Get("specularGlossinessTexture").Has("index")) {
                                    int specularTexIndex = pbrSG.Get("specularGlossinessTexture").Get("index").Get<int>();
                                    if (specularTexIndex >= 0 && specularTexIndex < gltfModel.textures.size()) {
                                        const auto& texture = gltfModel.textures[specularTexIndex];
                                        const auto& image = gltfModel.images[texture.source];
                                        if (!image.image.empty()) {
                                            try {
                                                auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                                material->SetSpecularGlossinessTexture(tex);
                                            } catch (const std::exception& e) {
                                                // 静默处理异常
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                    
                    if (!hasSpecularGlossiness) {
                        // Metallic-Roughness 工作流
                        material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
                        
                        const auto& pbr = gltfMaterial.pbrMetallicRoughness;
                        material->baseColor = glm::vec3(
                            pbr.baseColorFactor[0],
                            pbr.baseColorFactor[1],
                            pbr.baseColorFactor[2]
                        );
                        material->metallicFactor = pbr.metallicFactor;
                        material->roughnessFactor = pbr.roughnessFactor;
                        
                        // 基础颜色纹理
                        if (pbr.baseColorTexture.index >= 0) {
                            const auto& texture = gltfModel.textures[pbr.baseColorTexture.index];
                            const auto& image = gltfModel.images[texture.source];
                            if (!image.image.empty()) {
                                try {
                                    auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                    material->SetBaseColorTexture(tex);
                                } catch (const std::exception& e) {
                                    // 静默处理异常
                                }
                            }
                        }
                        
                        // 金属度粗糙度纹理
                        if (pbr.metallicRoughnessTexture.index >= 0) {
                            const auto& texture = gltfModel.textures[pbr.metallicRoughnessTexture.index];
                            const auto& image = gltfModel.images[texture.source];
                            if (!image.image.empty()) {
                                try {
                                    auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                    material->SetMetallicRoughnessTexture(tex);
                                } catch (const std::exception& e) {
                                    // 静默处理异常
                                }
                            }
                        }
                    }
                    
                    // 通用纹理
                    if (gltfMaterial.normalTexture.index >= 0) {
                        material->normalScale = gltfMaterial.normalTexture.scale;
                        const auto& texture = gltfModel.textures[gltfMaterial.normalTexture.index];
                        const auto& image = gltfModel.images[texture.source];
                        if (!image.image.empty()) {
                            try {
                                auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                material->SetNormalTexture(tex);
                            } catch (const std::exception& e) {
                                // 静默处理异常
                            }
                        }
                    }
                    
                    if (gltfMaterial.occlusionTexture.index >= 0) {
                        material->occlusionStrength = gltfMaterial.occlusionTexture.strength;
                        const auto& texture = gltfModel.textures[gltfMaterial.occlusionTexture.index];
                        const auto& image = gltfModel.images[texture.source];
                        if (!image.image.empty()) {
                            try {
                                auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                material->SetOcclusionTexture(tex);
                            } catch (const std::exception& e) {
                                // 静默处理异常
                            }
                        }
                    }
                    
                    if (gltfMaterial.emissiveTexture.index >= 0) {
                        const auto& texture = gltfModel.textures[gltfMaterial.emissiveTexture.index];
                        const auto& image = gltfModel.images[texture.source];
                        if (!image.image.empty()) {
                            try {
                                auto tex = std::make_shared<Texture>(image.image, image.width, image.height, false, false);
                                material->SetEmissionTexture(tex);
                            } catch (const std::exception& e) {
                                // 静默处理异常
                            }
                        }
                    }
                    
                    material->emissiveColor = glm::vec3(
                        gltfMaterial.emissiveFactor[0],
                        gltfMaterial.emissiveFactor[1],
                        gltfMaterial.emissiveFactor[2]
                    );
                    
                    if (!material->HasAnyTexture()) {
                        material->workflow = MaterialWorkflow::NO_TEXTURE;
                    }
                } else {
                    material->workflow = MaterialWorkflow::NO_TEXTURE;
                }
                
                // 创建网格
                if (!vertices.empty()) {
                    if (indices.empty()) {
                        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, m_DrawMode), material});
                    } else {
                        m_Meshes.emplace_back(TexturedMesh{Mesh(vertices, indices, m_DrawMode), material});
                    }
                }
            }
        }
    }

    // OBJ 加载保持类似逻辑，但创建 Material 而不是纹理列表
    void Model::LoadObjModel(const std::string& path, bool useSRGB) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        std::string baseDir = std::filesystem::path(path).parent_path().string();
        if (!baseDir.empty())
            baseDir += std::filesystem::path::preferred_separator;
        
        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                        path.c_str(), baseDir.c_str());
        if (!warn.empty()) std::cout << "[ModelLoader Warning] " << warn << std::endl;
        if (!err.empty()) std::cerr << "[ModelLoader Error] " << err << std::endl;
        if (!success) throw std::runtime_error("Failed to load model: " + path);
        
        m_Directory = baseDir;
        for (const auto& shape : shapes) {
            ProcessMeshData(&attrib, &shape, materials.data(), materials.size(), useSRGB);
        }
    }

    void Model::ProcessMeshData(const void* attribPtr,
                                const void* shapePtr,
                                const void* materialsPtr,
                                size_t materialCount,
                                bool useSRGB) {
        const tinyobj::attrib_t* attrib = static_cast<const tinyobj::attrib_t*>(attribPtr);
        const tinyobj::shape_t* shape = static_cast<const tinyobj::shape_t*>(shapePtr);
        const tinyobj::material_t* materials = static_cast<const tinyobj::material_t*>(materialsPtr);
        
        // 按材质分组处理（逻辑保持不变）
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
        
        // 为每种材质创建 Material 对象
        for (const auto& [matID, indices] : matID_to_indices) {
            auto material = std::make_shared<Material>();
            material->workflow = MaterialWorkflow::METALLIC_ROUGHNESS; // OBJ 默认使用 MR 工作流
            
            if (matID >= 0 && matID < static_cast<int>(materialCount)) {
                const auto& mat = materials[matID];
                
                // 设置材质颜色
                material->baseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
                
                // 加载纹理
                if (!mat.diffuse_texname.empty()) {
                    if (auto tex = LoadMaterialTexture(m_Directory, mat.diffuse_texname, useSRGB)) {
                        material->SetBaseColorTexture(tex);
                    }
                }
                if (!mat.bump_texname.empty()) {
                    if (auto tex = LoadMaterialTexture(m_Directory, mat.bump_texname, false)) { // 法线贴图不用 sRGB
                        material->SetNormalTexture(tex);
                    }
                }
                
                if (!material->HasAnyTexture()) {
                    material->workflow = MaterialWorkflow::NO_TEXTURE;
                }
            } else {
                material->workflow = MaterialWorkflow::NO_TEXTURE;
            }
            
            m_Meshes.emplace_back(TexturedMesh{Mesh(matID_to_vertices[matID], indices, m_DrawMode), material});
        }
    }

    std::shared_ptr<Texture> Model::LoadMaterialTexture(const std::string& baseDir,
                                                        const std::string& texPath,
                                                        bool useSRGB) {
        std::filesystem::path fullPath = std::filesystem::path(baseDir) / texPath;
        std::string fullPathStr = fullPath.string();
        
        try {
            auto tex = std::make_shared<Texture>(fullPathStr, useSRGB, false);
            return tex;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load texture: " << fullPathStr << "\nReason: " << e.what() << std::endl;
            return nullptr;
        }
    }

} // namespace graphics