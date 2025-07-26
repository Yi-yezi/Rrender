#include "pipeline/PointShadowMappingPipeline.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace pipeline {

PointShadowMappingPipeline::PointShadowMappingPipeline(std::shared_ptr<graphics::Shader> depthShader,
                                                       std::shared_ptr<graphics::Shader> mainShader,
                                                       int shadowSize)
    : m_DepthShader(std::move(depthShader)),
      m_MainShader(std::move(mainShader)),
      m_ShadowSize(shadowSize) {
    glEnable(GL_DEPTH_TEST);
    glGenTextures(1, &m_DepthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     m_ShadowSize, m_ShadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &m_DepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_MainShader->Bind();
    m_MainShader->SetUniform("u_DiffuseTexture", 0);
    m_MainShader->SetUniform("u_PointShadowMap", 15);

    m_FarPlane=25.0f; // 设置远平面距离
}


void PointShadowMappingPipeline::RenderDepthPass(const std::shared_ptr<scene::Scene>& scene) {
    glViewport(0, 0, m_ShadowSize, m_ShadowSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // 获取第一个点光源
    const auto& lights = scene->GetLights();
    if (lights.empty() || lights[0]->GetType() != graphics::LightType::Point)
        return;
    auto light = std::dynamic_pointer_cast<graphics::PointLight>(lights[0]);
    m_LightPos = light->GetPosition();

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, m_FarPlane);
    std::vector<glm::mat4> shadowTransforms = {
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
        shadowProj * glm::lookAt(m_LightPos, m_LightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
    };

    m_DepthShader->Bind();
    for (int i = 0; i < 6; ++i)
        m_DepthShader->SetUniform("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);

    m_DepthShader->SetUniform("u_PointLightFarPlane", m_FarPlane);
    m_DepthShader->SetUniform("u_PointLightPos", m_LightPos);

    for (const auto& entity : scene->GetEntities()) {
        m_DepthShader->SetUniform("u_Model", entity->GetModelMatrix());
        //entity->Draw(m_MainShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_DepthShader->Unbind();
}

void PointShadowMappingPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                                        const std::shared_ptr<graphics::Camera>& camera) {
    
    RenderDepthPass(scene);

    glViewport(0, 0, 1280, 720);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_MainShader->Bind();
    m_MainShader->SetUniform("u_View", camera->GetViewMatrix());
    m_MainShader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_MainShader->SetUniform("u_CameraPos", camera->GetPosition());
    m_MainShader->SetUniform("u_PointLightFarPlane", m_FarPlane);

    const auto& lights = scene->GetLights();
    int lightCount = std::min(static_cast<int>(lights.size()), 4);
    m_MainShader->SetUniform("u_LightCount", lightCount);

    using LightType = graphics::LightType;

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
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);
    for (const auto& entity : scene->GetEntities()) {
        m_MainShader->SetUniform("u_Model", entity->GetModelMatrix());
        //entity->Draw(m_MainShader);
    }

    m_MainShader->Unbind();
}

} // namespace pipeline
