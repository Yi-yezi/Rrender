#include "pipeline/OutlinePipeline.h"
#include <glm/gtc/matrix_transform.hpp>


namespace pipeline {

OutlinePipeline::OutlinePipeline(std::shared_ptr<graphics::Shader> baseShader,
                                 std::shared_ptr<graphics::Shader> outlineShader)
    : m_baseShader(std::move(baseShader)), m_outlineShader(std::move(outlineShader)) {}

void OutlinePipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                             const std::shared_ptr<graphics::Camera>& camera) {
    if (!scene || !camera || !m_baseShader || !m_outlineShader) return;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // 第一步：正常渲染，模板缓冲写1
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
                                
    m_baseShader->Bind();
    m_baseShader->SetUniform("u_View", camera->GetViewMatrix());
    m_baseShader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_baseShader->SetUniform("u_CameraPos", camera->GetPosition());

    const auto& lights = scene->GetLights();
    int lightCount = std::min(static_cast<int>(lights.size()), 4);
    m_baseShader->SetUniform("u_LightCount", lightCount);

    using LightType = graphics::Light::Type;

    for (int i = 0; i < lightCount; ++i) {
        const auto& light = lights[i];
        std::string base = "u_Lights[" + std::to_string(i) + "]";

        m_baseShader->SetUniform(base + ".type", static_cast<int>(light->GetType()));
        m_baseShader->SetUniform(base + ".color", light->GetColor());
        m_baseShader->SetUniform(base + ".intensity", light->GetIntensity());

        switch (light->GetType()) {
            case LightType::Directional: {
                auto* dirLight = dynamic_cast<graphics::DirectionalLight*>(light.get());
                if (dirLight)
                    m_baseShader->SetUniform(base + ".direction", dirLight->GetDirection());
                break;
            }
            case LightType::Point: {
                auto* pointLight = dynamic_cast<graphics::PointLight*>(light.get());
                if (pointLight) {
                    m_baseShader->SetUniform(base + ".position", pointLight->GetPosition());
                    m_baseShader->SetUniform(base + ".constant", pointLight->GetConstant());
                    m_baseShader->SetUniform(base + ".linear", pointLight->GetLinear());
                    m_baseShader->SetUniform(base + ".quadratic", pointLight->GetQuadratic());
                }
                break;
            }
            case LightType::Spot: {
                auto* spotLight = dynamic_cast<graphics::SpotLight*>(light.get());
                if (spotLight) {
                    m_baseShader->SetUniform(base + ".position", spotLight->GetPosition());
                    m_baseShader->SetUniform(base + ".direction", spotLight->GetDirection());
                    m_baseShader->SetUniform(base + ".constant", spotLight->GetConstant());
                    m_baseShader->SetUniform(base + ".linear", spotLight->GetLinear());
                    m_baseShader->SetUniform(base + ".quadratic", spotLight->GetQuadratic());
                    m_baseShader->SetUniform(base + ".innerCutOff", spotLight->GetInnerCutOff());
                    m_baseShader->SetUniform(base + ".outerCutOff", spotLight->GetOuterCutOff());
                }
                break;
            }
        }
    }

    for (const auto& entity : scene->GetEntities()) {
        m_baseShader->SetUniform("u_Model", entity->GetModelMatrix());
        entity->Draw();
    }

    m_baseShader->Unbind();

    // 第二步：绘制放大轮廓，仅模板不为1区域绘制
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    m_outlineShader->Bind();
    m_outlineShader->SetUniform("u_View", camera->GetViewMatrix());
    m_outlineShader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_outlineShader->SetUniform("u_OutlineColor", glm::vec3(0.04f, 0.28f, 0.26f)); // 轮廓颜色，可以改

    const float scale = 1.05f; // 放大比例
    for (const auto& entity : scene->GetEntities()) {
        glm::mat4 scaledModel = glm::scale(entity->GetModelMatrix(), glm::vec3(scale));
        m_outlineShader->SetUniform("u_Model", scaledModel);
        entity->Draw();
    }

    // 恢复状态
    glStencilMask(0xFF);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    m_baseShader->Unbind();
    m_outlineShader->Unbind();
}

} // namespace pipeline
