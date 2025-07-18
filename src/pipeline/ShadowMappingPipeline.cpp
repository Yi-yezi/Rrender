#include "pipeline/ShadowMappingPipeline.h"
#include <glad/glad.h>
#include <iostream>

namespace pipeline {

ShadowMappingPipeline::ShadowMappingPipeline(
    std::shared_ptr<graphics::Shader> depthShader,
    std::shared_ptr<graphics::Shader> mainShader,
    int shadowWidth, int shadowHeight)
    : m_DepthShader(std::move(depthShader)),
      m_MainShader(std::move(mainShader)),
      m_ShadowWidth(shadowWidth), m_ShadowHeight(shadowHeight) {

    glEnable(GL_DEPTH_TEST);
    // 创建深度图FBO
    glGenFramebuffers(1, &m_DepthFBO);

    glGenTextures(1, &m_DepthMap);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 m_ShadowWidth, m_ShadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };  // 表示无阴影
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMap, 0);
    glDrawBuffer(GL_NONE); 
    glReadBuffer(GL_NONE); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    m_MainShader->Bind();
    m_MainShader->SetUniform("u_DiffuseTexture", 0);
    m_MainShader->SetUniform("u_ShadowMap", 15);

}



void ShadowMappingPipeline::RenderDepthPass(const std::shared_ptr<scene::Scene>& scene) {
    glViewport(0, 0, m_ShadowWidth, m_ShadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    m_DepthShader->Bind();
    // 使用场景中的方向光
    const auto& lights = scene->GetLights();
    if (!lights.empty() && lights[0]->GetType() == graphics::Light::Type::Directional) {
        auto dirLight = std::dynamic_pointer_cast<graphics::DirectionalLight>(lights[0]);
        glm::vec3 lightDir = normalize(dirLight->GetDirection());
        glm::vec3 lightPos = -lightDir*4.5f;
        
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m_LightSpaceMatrix = lightProjection * lightView;
        
    }
    m_DepthShader->SetUniform("u_LightVP", m_LightSpaceMatrix);


    for (const auto& entity : scene->GetEntities()) {
        m_DepthShader->SetUniform("u_Model", entity->GetModelMatrix());
        entity->Draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_DepthShader->Unbind();
}

void ShadowMappingPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                                   const std::shared_ptr<graphics::Camera>& camera) {
    if (!m_DepthShader || !m_MainShader || !scene || !camera) return;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 第一遍：生成深度贴图
    glCullFace(GL_FRONT);
    RenderDepthPass(scene);
    glCullFace(GL_BACK);
                              
    // 第二遍：正常渲染，使用阴影贴图
    glViewport(0, 0, 1280, 720); // 恢复默认视口大小  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                    
    m_MainShader->Bind();

    m_MainShader->SetUniform("u_LightVP", m_LightSpaceMatrix);
    m_MainShader->SetUniform("u_View", camera->GetViewMatrix());
    m_MainShader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_MainShader->SetUniform("u_CameraPos", camera->GetPosition());

    const auto& lights = scene->GetLights();
    int lightCount = std::min(static_cast<int>(lights.size()), 4);
    m_MainShader->SetUniform("u_LightCount", lightCount);

    using LightType = graphics::Light::Type;

    for (int i = 0; i < lightCount; ++i) {
        const auto& light = lights[i];
        std::string base = "u_Lights[" + std::to_string(i) + "]";

        m_MainShader->SetUniform(base + ".type", static_cast<int>(light->GetType()));
        m_MainShader->SetUniform(base + ".color", light->GetColor());
        m_MainShader->SetUniform(base + ".intensity", light->GetIntensity());

        switch (light->GetType()) {
            case LightType::Directional: {
                auto* dirLight = dynamic_cast<graphics::DirectionalLight*>(light.get());
                if (dirLight)
                    m_MainShader->SetUniform(base + ".direction", dirLight->GetDirection());
                break;
            }
            case LightType::Point: {
                auto* pointLight = dynamic_cast<graphics::PointLight*>(light.get());
                if (pointLight) {
                    m_MainShader->SetUniform(base + ".position", pointLight->GetPosition());
                    m_MainShader->SetUniform(base + ".constant", pointLight->GetConstant());
                    m_MainShader->SetUniform(base + ".linear", pointLight->GetLinear());
                    m_MainShader->SetUniform(base + ".quadratic", pointLight->GetQuadratic());
                }
                break;
            }
            case LightType::Spot: {
                auto* spotLight = dynamic_cast<graphics::SpotLight*>(light.get());
                if (spotLight) {
                    m_MainShader->SetUniform(base + ".position", spotLight->GetPosition());
                    m_MainShader->SetUniform(base + ".direction", spotLight->GetDirection());
                    m_MainShader->SetUniform(base + ".constant", spotLight->GetConstant());
                    m_MainShader->SetUniform(base + ".linear", spotLight->GetLinear());
                    m_MainShader->SetUniform(base + ".quadratic", spotLight->GetQuadratic());
                    m_MainShader->SetUniform(base + ".innerCutOff", spotLight->GetInnerCutOff());
                    m_MainShader->SetUniform(base + ".outerCutOff", spotLight->GetOuterCutOff());
                }
                break;
            }
        }
    }



    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, m_DepthMap);
    for (const auto& entity : scene->GetEntities()) {
        m_MainShader->SetUniform("u_Model", entity->GetModelMatrix());

        entity->Draw();
    }

    m_MainShader->Unbind();
    
}

} // namespace pipeline
