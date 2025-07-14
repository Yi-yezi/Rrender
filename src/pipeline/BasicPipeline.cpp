#include "pipeline/BasicPipeline.h"
#include <glad/glad.h>

namespace pipeline {

BasicPipeline::BasicPipeline(std::shared_ptr<graphics::Shader> shader)
    : m_Shader(std::move(shader)) {}

void BasicPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                           const std::shared_ptr<graphics::Camera>& camera) {
    if (!m_Shader || !scene || !camera) return;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_Shader->Bind();

    // 传递视图和投影矩阵
    m_Shader->SetUniform("u_View", camera->GetViewMatrix());
    m_Shader->SetUniform("u_Projection", camera->GetProjectionMatrix());

    for (const auto& entity : scene->GetEntities()) {
        m_Shader->SetUniform("u_Model", entity->GetModelMatrix());
        entity->Draw();
    }

    m_Shader->Unbind();
}

} // namespace pipeline
