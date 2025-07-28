#include "renderPass/ShadowPass.h"
#include "resource/ResourceManager.h"
#include "assets/VerticesData.h"
#include "utils/PathResolver.h"
#include <iostream>


namespace pipeline {

ShadowPass::ShadowPass(std::shared_ptr<core::Window> windowPtr)
    : m_WindowPtr(std::move(windowPtr))
{
    // Directional shadow map (2D array)
    glGenFramebuffers(1, &m_DirectionalShadowFBO);
    glGenTextures(1, &m_DirectionalShadowMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_DirectionalShadowMap);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 m_2DShadowResolution, m_2DShadowResolution, m_MaxDirectionalLights, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Point shadow map (cube array)
    glGenFramebuffers(1, &m_PointShadowFBO);
    glGenTextures(1, &m_PointShadowMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_PointShadowMap);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 m_CubeShadowResolution, m_CubeShadowResolution, 6 * m_MaxPointLights, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Spot shadow map (2D array)
    glGenFramebuffers(1, &m_SpotShadowFBO);
    glGenTextures(1, &m_SpotShadowMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_SpotShadowMap);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                 m_2DShadowResolution, m_2DShadowResolution, m_MaxSpotLights, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);


    // setup quad VAO and VBO
    glGenBuffers(1, &m_quadVBO);
    glGenVertexArrays(1, &m_quadVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 加载阴影着色器（假设ResourceManager已实现）
    m_DirectionalShadowShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/shadow_directional.vert"), PathResolver::Resolve("shaders/alpha/shadow_directional.frag"));
    m_PointShadowShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/shadow_point.vert"), PathResolver::Resolve("shaders/alpha/shadow_point.frag"), PathResolver::Resolve("shaders/alpha/shadow_point.geom"));
    m_SpotShadowShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/shadow_spot.vert"), PathResolver::Resolve("shaders/alpha/shadow_spot.frag"));
    m_QuadShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/depth_quad.vert"), PathResolver::Resolve("shaders/alpha/depth_quad.frag"));
}

ShadowPass::~ShadowPass() {
    glDeleteFramebuffers(1, &m_DirectionalShadowFBO);
    glDeleteFramebuffers(1, &m_PointShadowFBO);
    glDeleteFramebuffers(1, &m_SpotShadowFBO);
    glDeleteTextures(1, &m_DirectionalShadowMap);
    glDeleteTextures(1, &m_PointShadowMap);
    glDeleteTextures(1, &m_SpotShadowMap);
}

GLuint ShadowPass::GetDirectionalShadowMap() const { return m_DirectionalShadowMap; }
GLuint ShadowPass::GetPointShadowMap() const { return m_PointShadowMap; }
GLuint ShadowPass::GetSpotShadowMap() const { return m_SpotShadowMap; }

void ShadowPass::Execute(const std::shared_ptr<scene::Scene>& scene) {
    // 设置OpenGL状态
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // 使用小于等于来处理阴影贴图
    if(!m_PointShdowRendered) {
        RenderPointShadows(scene);
        m_PointShdowRendered = true;
    } 
    RenderDirectionalShadows(scene);
    RenderSpotShadows(scene);
}

void ShadowPass::RenderDirectionalShadows(const std::shared_ptr<scene::Scene>& scene) {
    glViewport(0, 0, m_2DShadowResolution, m_2DShadowResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DirectionalShadowFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    m_DirectionalShadowShader->Bind();
    unsigned int dirlightCount = 0;
    const auto& lights = scene->GetLights();
    for(auto & light : lights) {
        if (light->GetType() != graphics::LightType::Directional) continue;
        if (dirlightCount >= m_MaxDirectionalLights) break;
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DirectionalShadowMap, 0, dirlightCount);
        glClear(GL_DEPTH_BUFFER_BIT);
        auto dirLight = std::dynamic_pointer_cast<graphics::DirectionalLight>(light);
        glm::mat4 lightVP = dirLight->GetLightVP();
        m_DirectionalShadowShader->SetUniform("u_lightVP", lightVP);
        for(auto& entity : scene->GetEntities()) {
            entity->Draw(m_DirectionalShadowShader);
        }
        dirlightCount++;
    }
    m_DirectionalShadowShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPass::RenderPointShadows(const std::shared_ptr<scene::Scene>& scene) {
    glViewport(0, 0, m_CubeShadowResolution, m_CubeShadowResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_PointShadowFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    m_PointShadowShader->Bind();
    unsigned int pointLightCount = 0;
    for (auto& light : scene->GetLights()) {
        if (light->GetType() != graphics::LightType::Point) continue;
        if (pointLightCount >= m_MaxPointLights) break;
        auto pointLight = std::dynamic_pointer_cast<graphics::PointLight>(light);
        glm::vec3 lightPos = pointLight->GetPosition();
        auto lightVPs= pointLight->GetLightVP(1.0f, 1.0f, 10.5f);
        for (int face = 0; face < 6; ++face) {
            int layer = pointLightCount * 6 + face;
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_PointShadowMap, 0, layer);
            glClear(GL_DEPTH_BUFFER_BIT);
            m_PointShadowShader->SetUniform("u_lightVP", lightVPs[face]);
            m_PointShadowShader->SetUniform("u_lightPos", lightPos);
            m_PointShadowShader->SetUniform("u_farPlane", 10.5f);
            for (auto& entity : scene->GetEntities()) {
                entity->Draw(m_PointShadowShader);
            }
        }
        pointLightCount++;
    }
    m_PointShadowShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPass::RenderSpotShadows(const std::shared_ptr<scene::Scene>& scene) {
    glViewport(0, 0, m_2DShadowResolution, m_2DShadowResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SpotShadowFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    m_SpotShadowShader->Bind();
    unsigned int spotLightCount = 0;
    for (auto& light : scene->GetLights()) {
        if (light->GetType() != graphics::LightType::Spot) continue;
        if (spotLightCount >= m_MaxSpotLights) break;
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_SpotShadowMap, 0, spotLightCount);
        glClear(GL_DEPTH_BUFFER_BIT);
        auto spotLight = std::dynamic_pointer_cast<graphics::SpotLight>(light);
        glm::mat4 lightVP = spotLight->GetLightVP();
        m_SpotShadowShader->SetUniform("u_lightVP", lightVP);
        for (auto& entity : scene->GetEntities()) {
            entity->Draw(m_SpotShadowShader);
        }
        spotLightCount++;
    }
    m_SpotShadowShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowPass::RenderQuad(){
    // 查看聚光灯阴影贴图
    int layer = 0; // 第一个聚光灯的阴影贴图层
    
    // 绑定聚光灯阴影贴图
    BindSpotShadowMap();
    
    m_QuadShader->Bind();
    // 设置为2D数组类型（聚光灯使用2D数组，不是立方体贴图）
    m_QuadShader->SetUniform("u_shadowMap", graphics::TextureSlots::TEX_SLOT_SHADOW_SPOT);
    m_QuadShader->SetUniform("u_layer", layer);
    m_QuadShader->SetUniform("u_face", 0);       // 2D数组不需要face，但保持兼容性
    m_QuadShader->SetUniform("u_mapType", 0);    // 重要：设置为0表示2D数组类型
    
    // 渲染四边形
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    m_QuadShader->Unbind();
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void ShadowPass::BindDirectionalShadowMap() const {
    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_SHADOW_DIR);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_DirectionalShadowMap);
}

void ShadowPass::BindPointShadowMap() const {
    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_SHADOW_POINT);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_PointShadowMap);
}

void ShadowPass::BindSpotShadowMap() const {
    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_SHADOW_SPOT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_SpotShadowMap);
}

} // namespace pipeline