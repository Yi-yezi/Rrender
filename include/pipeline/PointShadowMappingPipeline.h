#pragma once

#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "scene/Scene.h"

namespace pipeline {

class PointShadowMappingPipeline {
public:
    PointShadowMappingPipeline(std::shared_ptr<graphics::Shader> depthShader,
                               std::shared_ptr<graphics::Shader> mainShader,
                               int shadowSize = 1024);

    void RenderDepthPass(const std::shared_ptr<scene::Scene>& scene);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera);

private:
    GLuint m_DepthCubemap = 0;
    GLuint m_DepthFBO = 0;
    int m_ShadowSize;
    float m_FarPlane = 25.0f;

    glm::vec3 m_LightPos;

    std::shared_ptr<graphics::Shader> m_DepthShader;
    std::shared_ptr<graphics::Shader> m_MainShader;
};

} // namespace pipeline
