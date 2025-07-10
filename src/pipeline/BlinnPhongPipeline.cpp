#include "pipeline/BlinnPhongPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "graphics/Light.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <string>

namespace pipeline {

BlinnPhongPipeline::BlinnPhongPipeline(std::shared_ptr<graphics::Shader> shader)
    : m_Shader(std::move(shader)) {}

void BlinnPhongPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                                const std::shared_ptr<graphics::Camera>& camera) {
    if (!m_Shader || !scene || !camera) return;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);                                 
    m_Shader->Bind();
    m_Shader->SetUniform("u_View", camera->GetViewMatrix());
    m_Shader->SetUniform("u_Projection", camera->GetProjectionMatrix());
    m_Shader->SetUniform("u_CameraPos", camera->GetPosition());

    const auto& lights = scene->GetLights();
    int lightCount = std::min(static_cast<int>(lights.size()), 4);
    m_Shader->SetUniform("u_LightCount", lightCount);

    using LightType = graphics::Light::Type;

    for (int i = 0; i < lightCount; ++i) {
        const auto& light = lights[i];
        std::string base = "u_Lights[" + std::to_string(i) + "]";

        m_Shader->SetUniform(base + ".type", static_cast<int>(light->GetType()));
        m_Shader->SetUniform(base + ".color", light->GetColor());
        m_Shader->SetUniform(base + ".intensity", light->GetIntensity());

        switch (light->GetType()) {
            case LightType::Directional: {
                auto* dirLight = dynamic_cast<graphics::DirectionalLight*>(light.get());
                if (dirLight)
                    m_Shader->SetUniform(base + ".direction", dirLight->GetDirection());
                break;
            }
            case LightType::Point: {
                auto* pointLight = dynamic_cast<graphics::PointLight*>(light.get());
                if (pointLight) {
                    m_Shader->SetUniform(base + ".position", pointLight->GetPosition());
                    m_Shader->SetUniform(base + ".constant", pointLight->GetConstant());
                    m_Shader->SetUniform(base + ".linear", pointLight->GetLinear());
                    m_Shader->SetUniform(base + ".quadratic", pointLight->GetQuadratic());
                }
                break;
            }
            case LightType::Spot: {
                auto* spotLight = dynamic_cast<graphics::SpotLight*>(light.get());
                if (spotLight) {
                    m_Shader->SetUniform(base + ".position", spotLight->GetPosition());
                    m_Shader->SetUniform(base + ".direction", spotLight->GetDirection());
                    m_Shader->SetUniform(base + ".constant", spotLight->GetConstant());
                    m_Shader->SetUniform(base + ".linear", spotLight->GetLinear());
                    m_Shader->SetUniform(base + ".quadratic", spotLight->GetQuadratic());
                    m_Shader->SetUniform(base + ".innerCutOff", spotLight->GetInnerCutOff());
                    m_Shader->SetUniform(base + ".outerCutOff", spotLight->GetOuterCutOff());
                }
                break;
            }
        }
    }

    for (const auto& entity : scene->GetEntities()) {
        m_Shader->SetUniform("u_Model", entity->GetModelMatrix());
        entity->Draw();
    }

    m_Shader->Unbind();
}

} // namespace pipeline
