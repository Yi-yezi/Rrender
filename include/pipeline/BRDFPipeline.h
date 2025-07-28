#pragma once
#include <memory>
#include "graphics/Shader.h"
#include "scene/Scene.h"
#include "graphics/Camera.h"
#include "renderPass/ShadowPass.h"
#include "renderPass/IBLPass.h"
#include "core/Window.h"

namespace pipeline {

class BRDFPipeline {
public:
    BRDFPipeline(std::shared_ptr<core::Window> windowPtr);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera,
                bool useIBL = false, bool useShadow = false);


private:
    std::shared_ptr<ShadowPass> m_ShadowPassPtr; ///< 阴影渲染管线
    std::shared_ptr<IBLPass> m_IBLPassPtr; ///< IBL渲染管线
    std::shared_ptr<graphics::Shader> m_BRDFShader; ///< BRDF着色器
    std::shared_ptr<graphics::Shader> m_BackgroundShader; ///< 背景着色器
    std::shared_ptr<graphics::Shader> m_DebugShader;
    int m_Width = 1280; ///< 窗口宽度
    int m_Height = 720; ///< 窗口高度
    unsigned int m_CubeVAO = 0; ///< 立方体VAO
    unsigned int m_CubeVBO = 0; ///< 立方体VBO
    void RenderCube();
    

    std::shared_ptr<core::Window> m_WindowPtr; ///< 窗口指针，用于获取大小
};

} // namespace pipeline
