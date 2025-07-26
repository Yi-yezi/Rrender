#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Texture.h"

namespace graphics {
    class Shader;

    enum class MaterialWorkflow {
        METALLIC_ROUGHNESS = 0,
        SPECULAR_GLOSSINESS = 1,
        NO_TEXTURE = 2
    };

    class Material {
    public:
        // 构造函数
        Material() = default;
        
        // 材质工作流
        MaterialWorkflow workflow = MaterialWorkflow::METALLIC_ROUGHNESS;
        
        // Metallic-Roughness 工作流参数 (glTF 2.0 标准)
        glm::vec3 baseColor = glm::vec3(1.0f);      // baseColorFactor
        float metallicFactor = 0.0f;                // metallicFactor
        float roughnessFactor = 1.0f;               // roughnessFactor
        
        // Specular-Glossiness 工作流参数 (KHR_materials_pbrSpecularGlossiness 扩展)
        glm::vec3 diffuseColor = glm::vec3(1.0f);   // diffuseFactor
        glm::vec3 specularColor = glm::vec3(1.0f);  // specularFactor
        float glossinessFactor = 1.0f;              // glossinessFactor
        
        // 通用材质参数 (glTF 2.0 标准)
        glm::vec3 emissiveColor = glm::vec3(0.0f);  // emissiveFactor
        float normalScale = 1.0f;                   // normalTexture.scale
        float occlusionStrength = 1.0f;             // occlusionTexture.strength
        
        // Alpha 模式 (glTF 2.0 标准)
        std::string alphaMode = "OPAQUE";           // alphaMode: "OPAQUE", "MASK", "BLEND"
        float alphaCutoff = 0.5f;                   // alphaCutoff
        
        // 纹理 (使用 glTF 2.0 标准命名)
        // Metallic-Roughness 工作流纹理
        std::shared_ptr<Texture> baseColorTexture = nullptr;         // pbrMetallicRoughness.baseColorTexture
        std::shared_ptr<Texture> metallicRoughnessTexture = nullptr; // pbrMetallicRoughness.metallicRoughnessTexture
        
        // Specular-Glossiness 工作流纹理 (KHR_materials_pbrSpecularGlossiness 扩展)
        std::shared_ptr<Texture> diffuseTexture = nullptr;           // extensions.KHR_materials_pbrSpecularGlossiness.diffuseTexture
        std::shared_ptr<Texture> specularGlossinessTexture = nullptr; // extensions.KHR_materials_pbrSpecularGlossiness.specularGlossinessTexture
        
        // 通用纹理 (glTF 2.0 标准)
        std::shared_ptr<Texture> normalTexture = nullptr;            // normalTexture
        std::shared_ptr<Texture> occlusionTexture = nullptr;         // occlusionTexture
        std::shared_ptr<Texture> emissiveTexture = nullptr;          // emissiveTexture
        
        // 核心方法
        void SetUniforms(std::shared_ptr<Shader> shader) const;
        void BindTextures(std::shared_ptr<Shader> shader) const;
        
        // 检查纹理是否存在
        bool HasAnyTexture() const;
        
        // 设置纹理的便捷方法 (使用 glTF 2.0 标准命名)
        void SetBaseColorTexture(std::shared_ptr<Texture> texture) { baseColorTexture = texture; }
        void SetMetallicRoughnessTexture(std::shared_ptr<Texture> texture) { metallicRoughnessTexture = texture; }
        void SetDiffuseTexture(std::shared_ptr<Texture> texture) { diffuseTexture = texture; }
        void SetSpecularGlossinessTexture(std::shared_ptr<Texture> texture) { specularGlossinessTexture = texture; }
        void SetNormalTexture(std::shared_ptr<Texture> texture) { normalTexture = texture; }
        void SetOcclusionTexture(std::shared_ptr<Texture> texture) { occlusionTexture = texture; }
        void SetEmissionTexture(std::shared_ptr<Texture> texture) { emissiveTexture = texture; }
    };

} // namespace graphics