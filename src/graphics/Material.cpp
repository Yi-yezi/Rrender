#include "graphics/Material.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"

namespace graphics {

    void Material::SetUniforms(std::shared_ptr<Shader> shader) const {
        // 设置工作流类型
        shader->SetUniform("u_materialWorkflow", static_cast<int>(workflow));
        
        // 根据工作流类型设置不同的 uniform
        switch (workflow) {
            case MaterialWorkflow::METALLIC_ROUGHNESS: {
                shader->SetUniform("u_baseColor", baseColor);
                shader->SetUniform("u_metallicFactor", metallicFactor);
                shader->SetUniform("u_roughnessFactor", roughnessFactor);
                
                // 设置纹理可用标志
                bool hasBaseColor = baseColorTexture != nullptr;
                bool hasMetallicRoughness = metallicRoughnessTexture != nullptr;
                
                shader->SetUniform("u_hasBaseColorTexture", hasBaseColor);
                shader->SetUniform("u_hasMetallicRoughnessTexture", hasMetallicRoughness);
                break;
            }
            
            case MaterialWorkflow::SPECULAR_GLOSSINESS: {
                shader->SetUniform("u_diffuseColor", diffuseColor);
                shader->SetUniform("u_specularColor", specularColor);
                shader->SetUniform("u_glossinessFactor", glossinessFactor);
                
                // 设置纹理可用标志
                bool hasDiffuse = diffuseTexture != nullptr;
                bool hasSpecularGlossiness = specularGlossinessTexture != nullptr;
                
                shader->SetUniform("u_hasDiffuseTexture", hasDiffuse);
                shader->SetUniform("u_hasSpecularGlossinessTexture", hasSpecularGlossiness);
                break;
            }
            
            case MaterialWorkflow::NO_TEXTURE: {
                // 纯颜色材质，根据原始工作流类型设置相应的颜色值
                shader->SetUniform("u_baseColor", baseColor);
                shader->SetUniform("u_metallicFactor", metallicFactor);
                shader->SetUniform("u_roughnessFactor", roughnessFactor);
                
                // 同时也设置 Specular-Glossiness 参数，以防 shader 需要
                shader->SetUniform("u_diffuseColor", diffuseColor);
                shader->SetUniform("u_specularColor", specularColor);
                shader->SetUniform("u_glossinessFactor", glossinessFactor);
                
                // 所有主要纹理标志设为 false
                shader->SetUniform("u_hasBaseColorTexture", false);
                shader->SetUniform("u_hasMetallicRoughnessTexture", false);
                shader->SetUniform("u_hasDiffuseTexture", false);
                shader->SetUniform("u_hasSpecularGlossinessTexture", false);
                break;
            }
        }
        
        // 设置通用材质参数（所有工作流都需要）
        shader->SetUniform("u_emissiveColor", emissiveColor);
        shader->SetUniform("u_normalScale", normalScale);
        shader->SetUniform("u_occlusionStrength", occlusionStrength);
        
        // 设置通用纹理标志（所有工作流都需要检查）
        bool hasNormal = normalTexture != nullptr;
        bool hasOcclusion = occlusionTexture != nullptr;
        bool hasEmission = emissiveTexture != nullptr;
        
        shader->SetUniform("u_hasNormalTexture", hasNormal);
        shader->SetUniform("u_hasOcclusionTexture", hasOcclusion);
        shader->SetUniform("u_hasEmissionTexture", hasEmission);
        
        // 设置 Alpha 模式（所有工作流都需要）
        if (alphaMode == "OPAQUE") {
            shader->SetUniform("u_alphaMode", 0);
        } else if (alphaMode == "MASK") {
            shader->SetUniform("u_alphaMode", 1);
            shader->SetUniform("u_alphaCutoff", alphaCutoff);
        } else if (alphaMode == "BLEND") {
            shader->SetUniform("u_alphaMode", 2);
        } else {
            shader->SetUniform("u_alphaMode", 0); // 默认为 OPAQUE
        }
    }

    void Material::BindTextures(std::shared_ptr<Shader> shader) const {
        // 根据工作流绑定主要纹理
        switch (workflow) {
            case MaterialWorkflow::METALLIC_ROUGHNESS: {
                if (baseColorTexture) {
                    baseColorTexture->Bind(TEX_SLOT_BASE_COLOR);
                    shader->SetUniform("u_baseColorMap", TEX_SLOT_BASE_COLOR);
                }
                
                if (metallicRoughnessTexture) {
                    metallicRoughnessTexture->Bind(TEX_SLOT_METALLIC_ROUGHNESS);
                    shader->SetUniform("u_metallicRoughnessMap", TEX_SLOT_METALLIC_ROUGHNESS);
                }
                break;
            }
            
            case MaterialWorkflow::SPECULAR_GLOSSINESS: {
                if (diffuseTexture) {
                    diffuseTexture->Bind(TEX_SLOT_DIFFUSE);
                    shader->SetUniform("u_baseColorMap", TEX_SLOT_DIFFUSE); // 复用 baseColor uniform
                }
                
                if (specularGlossinessTexture) {
                    specularGlossinessTexture->Bind(TEX_SLOT_SPECULAR_GLOSSINESS);
                    shader->SetUniform("u_specularGlossinessMap", TEX_SLOT_SPECULAR_GLOSSINESS);
                }
                break;
            }
            
            case MaterialWorkflow::NO_TEXTURE: {
                // 无纹理工作流不需要绑定主要纹理
                break;
            }
        }
        
        // 绑定通用纹理（所有工作流都可能需要）
        if (normalTexture) {
            normalTexture->Bind(TEX_SLOT_NORMAL);
            shader->SetUniform("u_normalMap", TEX_SLOT_NORMAL);
        }
        
        if (occlusionTexture) {
            occlusionTexture->Bind(TEX_SLOT_OCCLUSION);
            shader->SetUniform("u_occlusionMap", TEX_SLOT_OCCLUSION);
        }
        
        if (emissiveTexture) {
            emissiveTexture->Bind(TEX_SLOT_EMISSION);
            shader->SetUniform("u_emissionMap", TEX_SLOT_EMISSION);
        }
    }

    bool Material::HasAnyTexture() const {
        return baseColorTexture || metallicRoughnessTexture ||
               diffuseTexture || specularGlossinessTexture ||
               normalTexture || occlusionTexture || emissiveTexture;
    }

} // namespace graphics