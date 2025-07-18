#pragma once
#include "pipeline/RenderPipeline.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "scene/Scene.h"
#include "core/Window.h"

namespace pipeline {

class DeferredShadingPipeline : public RenderPipeline {
public:
    DeferredShadingPipeline(std::shared_ptr<graphics::Shader> geometryShader,
                            std::shared_ptr<graphics::Shader> lightingShader,
                            std::shared_ptr<core::Window> windowPtr);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;

private:
    std::shared_ptr<graphics::Shader> m_GeometryShader;
    std::shared_ptr<graphics::Shader> m_LightingShader;
    std::shared_ptr<core::Window> m_WindowPtr;

    unsigned int m_GBuffer;
    unsigned int m_GPosition, m_GNormal, m_GAlbedoSpec;
    unsigned int m_RBO;

    unsigned int m_quadVAO, m_quadVBO;

    int m_Width, m_Height;

    void InitQuad();
};

} // namespace pipeline
