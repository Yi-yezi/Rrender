#pragma once
#include "pipeline/RenderPipeline.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "core/Window.h"
#include "scene/Scene.h"

namespace pipeline {

class PBRPipeline : public RenderPipeline {
public:
    explicit PBRPipeline(std::shared_ptr<graphics::Shader> shader,std::shared_ptr<core::Window> windowPtr);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;

private:
    std::shared_ptr<graphics::Shader> m_Shader;
    std::shared_ptr<core::Window> m_WindowPtr;
    int m_Width = 1280;  ///< 窗口宽度
    int m_Height = 720; ///< 窗口高度
    unsigned int m_sphereVAO = 0;
    unsigned int m_indexCount;
    void RenderSphere();
};

} // namespace pipeline
