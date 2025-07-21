#pragma once
#include "pipeline/RenderPipeline.h"
#include "core/Window.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "scene/Scene.h"

namespace pipeline {

class HDRPipeline : public RenderPipeline {
public:
    HDRPipeline(std::shared_ptr<graphics::Shader> depthShader,
                          std::shared_ptr<graphics::Shader> mainShader,std::shared_ptr<core::Window> windowPtr);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;

private:
        

    std::shared_ptr<graphics::Shader> m_HDRShader;
    std::shared_ptr<graphics::Shader> m_MainShader;

    unsigned int m_HDRFBO;
    unsigned int m_ColorBuffer;  // RGBA16F
    unsigned int m_RBO;          // 深度和模板缓冲
    std::shared_ptr<core::Window> m_WindowPtr;  ///< 窗口指针，用于获取大小
    int m_Width, m_Height;  ///< 窗口宽高
    unsigned int m_quadVAO;  ///< 全屏四边形的 VAO
    unsigned int m_quadVBO;  ///< 全屏四边形的 VBO

};

} // namespace pipeline