#pragma once
#include "renderPass/RenderPass.h"
#include "graphics/Camera.h"
#include "scene/Scene.h"
#include "core/Window.h"
#include "graphics/Shader.h"

namespace pipeline {

class ShadowPass : public RenderPass {
public:
    ShadowPass(std::shared_ptr<core::Window> windowPtr);

    void Execute(const std::shared_ptr<scene::Scene>& scene=nullptr) override;

    void RenderQuad();
    void BindDirectionalShadowMap() const;
    void BindPointShadowMap() const;
    void BindSpotShadowMap() const;

    
    GLuint GetDirectionalShadowMap() const;
    GLuint GetPointShadowMap() const;
    GLuint GetSpotShadowMap() const;

    ~ShadowPass() override;

private:
    void RenderDirectionalShadows(const std::shared_ptr<scene::Scene>& scene);
    void RenderPointShadows(const std::shared_ptr<scene::Scene>& scene);
    void RenderSpotShadows(const std::shared_ptr<scene::Scene>& scene);
    

    // FBO和纹理
    GLuint m_DirectionalShadowFBO = 0; ///< 平行光阴影FBO
    GLuint m_PointShadowFBO = 0; ///< 点光源阴影FBO
    GLuint m_SpotShadowFBO = 0; ///< 聚光灯阴影FBO

    // shadow map纹理
    GLuint m_DirectionalShadowMap = 0; ///< 平行光阴影贴图
    GLuint m_PointShadowMap = 0; ///< 点光源阴影贴图
    GLuint m_SpotShadowMap = 0; ///< 聚光灯阴影贴图

    const unsigned int m_2DShadowResolution = 1024; ///< 2D阴影贴图分辨率
    const unsigned int m_CubeShadowResolution = 512; ///< 立方体阴影贴图分辨率
    const unsigned int m_MaxDirectionalLights = 4; ///< 最大平行光数量
    const unsigned int m_MaxPointLights = 4; ///< 最大点光源数量
    const unsigned int m_MaxSpotLights = 4; ///< 最大聚光灯数量

    GLuint m_quadVAO = 0; ///< 用于渲染阴影的四边形VAO
    GLuint m_quadVBO = 0; ///< 用于渲染阴影的四边形VBO



    std::shared_ptr<graphics::Shader> m_DirectionalShadowShader; ///< 平行光阴影着色器
    std::shared_ptr<graphics::Shader> m_PointShadowShader; ///< 点光源
    std::shared_ptr<graphics::Shader> m_SpotShadowShader; ///< 聚光灯阴影着色器
    std::shared_ptr<graphics::Shader> m_QuadShader; ///< 用于渲染阴影的四边形着色器
    std::shared_ptr<core::Window> m_WindowPtr; ///< 窗口
};

} // namespace pipeline