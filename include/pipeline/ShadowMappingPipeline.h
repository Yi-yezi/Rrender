#pragma once
#include "pipeline/RenderPipeline.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "scene/Scene.h"

namespace pipeline {

class ShadowMappingPipeline : public RenderPipeline {
public:
    ShadowMappingPipeline(std::shared_ptr<graphics::Shader> depthShader,
                          std::shared_ptr<graphics::Shader> mainShader,int shadowWidth=1024, int shadowHeight=1024);


    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;


private:
    void RenderDepthPass(const std::shared_ptr<scene::Scene>& scene);

    std::shared_ptr<graphics::Shader> m_DepthShader;
    std::shared_ptr<graphics::Shader> m_MainShader;

    GLuint m_DepthFBO = 0;
    GLuint m_DepthMap = 0;

    int m_ShadowWidth, m_ShadowHeight;
    glm::mat4 m_LightSpaceMatrix;
};

} // namespace pipeline
