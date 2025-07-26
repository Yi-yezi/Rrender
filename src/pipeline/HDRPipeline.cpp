#include "pipeline/HDRPipeline.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include "graphics/Mesh.h" // 如果你有一个 ScreenQuad

namespace pipeline {

HDRPipeline::HDRPipeline(std::shared_ptr<graphics::Shader> hdrShader,
                         std::shared_ptr<graphics::Shader> mainShader,
                         std::shared_ptr<core::Window> windowPtr)
    : m_HDRShader(std::move(hdrShader)),
      m_MainShader(std::move(mainShader)),
      m_WindowPtr(std::move(windowPtr)) {

    glEnable(GL_DEPTH_TEST);
    m_WindowPtr->GetSize(m_Width, m_Height);
    // 创建HDR帧缓冲
    glGenFramebuffers(1, &m_HDRFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);

    // 创建浮点颜色缓冲
    glGenTextures(1, &m_ColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_ColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, nullptr); // 使用浮点格式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorBuffer, 0);

    // 创建深度缓冲
    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[HDRPipeline] HDR framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_HDRShader->Bind();
    m_HDRShader->SetUniform("u_HDRBuffer", 0);
    m_HDRShader->SetUniform("u_Exposure", 1.0f); // 默认曝光值


    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // setup plane VAO
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void HDRPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                         const std::shared_ptr<graphics::Camera>& camera) {
    // Pass 1: 场景渲染到 HDR FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
    m_WindowPtr->GetSize(m_Width, m_Height);
    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_MainShader->Bind();
    m_MainShader->SetUniform("u_View", camera->GetViewMatrix());
    m_MainShader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_MainShader->SetUniform("u_CameraPos", camera->GetPosition());

    // 传递光源信息
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

    for (const auto& entity : scene->GetEntities()) {
        m_MainShader->SetUniform("u_Model", entity->GetModelMatrix());
        //entity->Draw();
    }
    m_MainShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Pass 2: 后处理（Tone Mapping）

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_HDRShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ColorBuffer);

    m_HDRShader->SetUniform("u_HDRBuffer", 0);
    m_HDRShader->SetUniform("u_Exposure", 1.0f); // 曝光值可以根据需要调整
    // 渲染全屏 quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    m_HDRShader->Unbind();

}

} // namespace pipeline
