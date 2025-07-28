#include "pipeline/BRDFPipeline.h"
#include "resource/ResourceManager.h"
#include "utils/PathResolver.h"
#include "scene/Scene.h"
#include "graphics/Camera.h"
#include "graphics/Light.h"
#include "assets/VerticesData.h"
#include <iostream>


namespace pipeline {

BRDFPipeline::BRDFPipeline(std::shared_ptr<core::Window> windowPtr) {
    m_WindowPtr = std::move(windowPtr);

    m_BRDFShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/BRDF.vert"),
        PathResolver::Resolve("shaders/alpha/BRDF.frag")
    );

    m_BackgroundShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/background.vert"),
        PathResolver::Resolve("shaders/alpha/background.frag")
    );

    m_IBLPassPtr = std::make_shared<IBLPass>(m_WindowPtr,
        PathResolver::Resolve("assets/textures/hdr/newport_loft.hdr"));

    m_ShadowPassPtr = std::make_shared<ShadowPass>(m_WindowPtr);


    m_IBLPassPtr->Execute(); // 预处理IBL贴图



    // setup cube VAO and VBO
    glGenBuffers(1, &m_CubeVBO);
    glGenVertexArrays(1, & m_CubeVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_CubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
}

void BRDFPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) {
    if (!scene || !camera) return;

    if(m_UseShadow)
        m_ShadowPassPtr->Execute(scene); // 预处理阴影贴图

    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    float renderWidth = m_WindowPtr->GetRenderWidth();
    float renderHeight = m_WindowPtr->GetRenderHeight();
    glViewport(0, 0, static_cast<int>(renderWidth), static_cast<int>(renderHeight));
    camera->SetAspectRatio(m_WindowPtr->GetAspectRatio());

    m_BRDFShader->Bind();

    // 统计光源数量并设置uniform
    int directionalLightCount = 0;
    int pointLightCount = 0;
    int spotLightCount = 0;

    m_BRDFShader->SetUniform("u_enableShadow", m_UseShadow);
    m_BRDFShader->SetUniform("u_enableIBL", m_UseIBL);

    // 设置光源属性 - 只处理启用的光源
    for (auto& light : scene->GetLights()) {
        // 跳过禁用的光源
        if (!light->IsEnabled()) {
            continue;
        }
        
        switch (light->GetType()) {
            case graphics::LightType::Directional: {
                if (directionalLightCount >= 4) break; // 防止超出着色器数组大小
                
                auto dirLight = std::dynamic_pointer_cast<graphics::DirectionalLight>(light);
                std::string prefix = "u_dirLights[" + std::to_string(directionalLightCount) + "]";
                
                // 应用光源强度到颜色
                glm::vec3 lightColor = dirLight->GetColor() * dirLight->GetIntensity();
                
                m_BRDFShader->SetUniform(prefix + ".direction", dirLight->GetDirection());
                m_BRDFShader->SetUniform(prefix + ".color", lightColor);
                m_BRDFShader->SetUniform(prefix + ".shadowLayer", directionalLightCount);
                
                // 方向光VP矩阵
                m_BRDFShader->SetUniform("u_dirLightVP[" + std::to_string(directionalLightCount) + "]", 
                                       dirLight->GetLightVP());
                directionalLightCount++;
                break;
            }
            case graphics::LightType::Point: {
                if (pointLightCount >= 4) break; // 防止超出着色器数组大小
                
                auto pointLight = std::dynamic_pointer_cast<graphics::PointLight>(light);
                std::string prefix = "u_pointLights[" + std::to_string(pointLightCount) + "]";
                
                // 应用光源强度到颜色
                glm::vec3 lightColor = pointLight->GetColor() * pointLight->GetIntensity();
                
                m_BRDFShader->SetUniform(prefix + ".position", pointLight->GetPosition());
                m_BRDFShader->SetUniform(prefix + ".color", lightColor);
                m_BRDFShader->SetUniform(prefix + ".constant", pointLight->GetConstant());
                m_BRDFShader->SetUniform(prefix + ".linear", pointLight->GetLinear());
                m_BRDFShader->SetUniform(prefix + ".quadratic", pointLight->GetQuadratic());
                m_BRDFShader->SetUniform(prefix + ".shadowLayer", pointLightCount);
                
                pointLightCount++;
                break;
            }
            case graphics::LightType::Spot: {
                if (spotLightCount >= 4) break; // 防止超出着色器数组大小
                
                auto spotLight = std::dynamic_pointer_cast<graphics::SpotLight>(light);
                std::string prefix = "u_spotLights[" + std::to_string(spotLightCount) + "]";
                
                // 应用光源强度到颜色
                glm::vec3 lightColor = spotLight->GetColor() * spotLight->GetIntensity();
                
                m_BRDFShader->SetUniform(prefix + ".position", spotLight->GetPosition());
                m_BRDFShader->SetUniform(prefix + ".direction", spotLight->GetDirection());
                m_BRDFShader->SetUniform(prefix + ".color", lightColor);
                m_BRDFShader->SetUniform(prefix + ".innerCutOff", spotLight->GetInnerCutOff());
                m_BRDFShader->SetUniform(prefix + ".outerCutOff", spotLight->GetOuterCutOff());
                m_BRDFShader->SetUniform(prefix + ".constant", spotLight->GetConstant());
                m_BRDFShader->SetUniform(prefix + ".linear", spotLight->GetLinear());
                m_BRDFShader->SetUniform(prefix + ".quadratic", spotLight->GetQuadratic());
                m_BRDFShader->SetUniform(prefix + ".shadowLayer", spotLightCount);
                
                // 聚光灯VP矩阵
                m_BRDFShader->SetUniform("u_spotLightVP[" + std::to_string(spotLightCount) + "]", 
                                       spotLight->GetLightVP());
                spotLightCount++;
                break;
            }
            default:
                break;
        }
    }

    // 设置实际启用的光源数量
    m_BRDFShader->SetUniform("u_dirLightCount", directionalLightCount);
    m_BRDFShader->SetUniform("u_pointLightCount", pointLightCount);
    m_BRDFShader->SetUniform("u_spotLightCount", spotLightCount);

    // 相机参数
    m_BRDFShader->SetUniform("u_camPos", camera->GetPosition());
    m_BRDFShader->SetUniform("u_view", camera->GetViewMatrix());
    m_BRDFShader->SetUniform("u_projection", camera->GetProjectionMatrix());

    // IBL 设置
    if (m_UseIBL) {
        m_IBLPassPtr->BindMap();
        m_BRDFShader->SetUniform("u_irradianceMap", graphics::TextureSlots::TEX_SLOT_IRRADIANCE);
        m_BRDFShader->SetUniform("u_prefilterMap", graphics::TextureSlots::TEX_SLOT_PREFILTER);
        m_BRDFShader->SetUniform("u_brdfLUT", graphics::TextureSlots::TEX_SLOT_BRDF_LUT);
    }

    // 阴影贴图设置（如果启用阴影）
    if (m_UseShadow) {
        m_ShadowPassPtr->BindDirectionalShadowMap();
        m_BRDFShader->SetUniform("u_dirShadowMap", graphics::TextureSlots::TEX_SLOT_SHADOW_DIR);
        m_ShadowPassPtr->BindPointShadowMap();
        m_BRDFShader->SetUniform("u_pointShadowMap", graphics::TextureSlots::TEX_SLOT_SHADOW_POINT);
        m_ShadowPassPtr->BindSpotShadowMap();
        m_BRDFShader->SetUniform("u_spotShadowMap", graphics::TextureSlots::TEX_SLOT_SHADOW_SPOT);
    }

    // 渲染场景中的所有实体
    for (auto& entity : scene->GetEntities()) {
        entity->Draw(m_BRDFShader);
    }

    m_BRDFShader->Unbind();

    // 渲染环境贴图背景
    if (m_UseIBL) {
        m_BackgroundShader->Bind();
        m_BackgroundShader->SetUniform("environmentMap", graphics::TextureSlots::TEX_SLOT_ENV_CUBEMAP);
        m_BackgroundShader->SetUniform("projection", camera->GetProjectionMatrix());
        m_BackgroundShader->SetUniform("view", camera->GetViewMatrix());
        RenderCube(); // render the cube with the background shader
        m_BackgroundShader->Unbind();
    }


    //m_ShadowPassPtr->RenderQuad(); // 渲染阴影四边形
}

void BRDFPipeline::RenderCube() {
    glBindVertexArray(m_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

bool BRDFPipeline::IsIBLEnabled() {
    return m_UseIBL;
}

bool BRDFPipeline::IsShadowEnabled() {
    return m_UseShadow;
}

void BRDFPipeline::SetIBL(bool enabled) {
    m_UseIBL = enabled;
}
void BRDFPipeline::SetShadow(bool enabled) {
    m_UseShadow = enabled;
}

} // namespace pipeline