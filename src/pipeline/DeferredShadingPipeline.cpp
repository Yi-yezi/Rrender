#include "pipeline/DeferredShadingPipeline.h"
#include <glad/glad.h>
#include <iostream>

namespace pipeline {

DeferredShadingPipeline::DeferredShadingPipeline(std::shared_ptr<graphics::Shader> geometryShader,
                                                 std::shared_ptr<graphics::Shader> lightingShader,
                                                 std::shared_ptr<core::Window> windowPtr)
    : m_GeometryShader(std::move(geometryShader)),
      m_LightingShader(std::move(lightingShader)),
      m_WindowPtr(std::move(windowPtr)) {

    glEnable(GL_DEPTH_TEST);
    m_WindowPtr->GetSize(m_Width, m_Height);

    // 初始化 G-Buffer
    glGenFramebuffers(1, &m_GBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);

    // Position texture
    glGenTextures(1, &m_GPosition);
    glBindTexture(GL_TEXTURE_2D, m_GPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GPosition, 0);

    // Normal texture
    glGenTextures(1, &m_GNormal);
    glBindTexture(GL_TEXTURE_2D, m_GNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GNormal, 0);

    // Albedo + Specular
    glGenTextures(1, &m_GAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, m_GAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GAlbedoSpec, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Depth Renderbuffer
    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Deferred] G-Buffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    InitQuad();
}

void DeferredShadingPipeline::InitQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void DeferredShadingPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                                     const std::shared_ptr<graphics::Camera>& camera) {
    // 1. Geometry Pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);
    glViewport(0, 0, m_Width, m_Height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_GeometryShader->Bind();
    m_GeometryShader->SetUniform("u_View", camera->GetViewMatrix());
    m_GeometryShader->SetUniform("u_Projection", camera->GetProjectionMatrix());

    for (const auto& entity : scene->GetEntities()) {
        m_GeometryShader->SetUniform("u_Model", entity->GetModelMatrix());
        entity->Draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Lighting Pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_LightingShader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_GAlbedoSpec);

    m_LightingShader->SetUniform("gPosition", 0);
    m_LightingShader->SetUniform("gNormal", 1);
    m_LightingShader->SetUniform("gAlbedoSpec", 2);
    m_LightingShader->SetUniform("u_CameraPos", camera->GetPosition());

    // 传递光源信息
    const auto& lights = scene->GetLights();
    int lightCount = std::min(static_cast<int>(lights.size()), 4);
    m_LightingShader->SetUniform("u_LightCount", lightCount);

    using LightType = graphics::Light::Type;

    for (int i = 0; i < lightCount; ++i) {
        const auto& light = lights[i];
        std::string base = "u_Lights[" + std::to_string(i) + "]";

        m_LightingShader->SetUniform(base + ".type", static_cast<int>(light->GetType()));
        m_LightingShader->SetUniform(base + ".color", light->GetColor());
        m_LightingShader->SetUniform(base + ".intensity", light->GetIntensity());

        switch (light->GetType()) {
            case LightType::Directional: {
                auto* dirLight = dynamic_cast<graphics::DirectionalLight*>(light.get());
                if (dirLight)
                    m_LightingShader->SetUniform(base + ".direction", dirLight->GetDirection());
                break;
            }
            case LightType::Point: {
                auto* pointLight = dynamic_cast<graphics::PointLight*>(light.get());
                if (pointLight) {
                    m_LightingShader->SetUniform(base + ".position", pointLight->GetPosition());
                    m_LightingShader->SetUniform(base + ".constant", pointLight->GetConstant());
                    m_LightingShader->SetUniform(base + ".linear", pointLight->GetLinear());
                    m_LightingShader->SetUniform(base + ".quadratic", pointLight->GetQuadratic());
                }
                break;
            }
            case LightType::Spot: {
                auto* spotLight = dynamic_cast<graphics::SpotLight*>(light.get());
                if (spotLight) {
                    m_LightingShader->SetUniform(base + ".position", spotLight->GetPosition());
                    m_LightingShader->SetUniform(base + ".direction", spotLight->GetDirection());
                    m_LightingShader->SetUniform(base + ".constant", spotLight->GetConstant());
                    m_LightingShader->SetUniform(base + ".linear", spotLight->GetLinear());
                    m_LightingShader->SetUniform(base + ".quadratic", spotLight->GetQuadratic());
                    m_LightingShader->SetUniform(base + ".innerCutOff", spotLight->GetInnerCutOff());
                    m_LightingShader->SetUniform(base + ".outerCutOff", spotLight->GetOuterCutOff());
                }
                break;
            }
        }
    }

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    m_LightingShader->Unbind();
}

} // namespace pipeline
