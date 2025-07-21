#pragma once
#include "pipeline/RenderPipeline.h"
#include "graphics/Shader.h"
#include "graphics/Camera.h"
#include "core/Window.h"
#include "scene/Scene.h"

namespace pipeline {

class IBLPipeline : public RenderPipeline {
public:
    IBLPipeline(std::shared_ptr<graphics::Shader> pbrShader,
                std::shared_ptr<graphics::Shader> irradianceShader,
                std::shared_ptr<graphics::Shader> prefilterShader,
                std::shared_ptr<graphics::Shader> brdfShader,
                std::shared_ptr<graphics::Shader> backgroundShader,
                std::shared_ptr<graphics::Shader> equirectangular2cubemapShader,
                const std::string& equirectangularMapPath,
                std::shared_ptr<core::Window> windowPtr);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;

    ~IBLPipeline() override;

private:
    void SetupEnvironmentMap(const std::string& equirectangularMapPath);
    void GenerateIrradianceMap();
    void GeneratePrefilterMap();
    void GenerateBRDFLUT();
    void RenderCube();
    void RenderQuad();


    std::shared_ptr<graphics::Shader> m_PBRShader;
    std::shared_ptr<graphics::Shader> m_IrradianceShader;
    std::shared_ptr<graphics::Shader> m_PrefilterShader;
    std::shared_ptr<graphics::Shader> m_BRDFShader;
    std::shared_ptr<graphics::Shader> m_BackgroundShader;
    std::shared_ptr<graphics::Shader> m_Equirectangular2CubemapShader;
    std::shared_ptr<core::Window> m_WindowPtr;

    int m_Width = 1280;
    int m_Height = 720;
    const glm::mat4 m_captureViews[6] = {
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)), // +X
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)), // -X
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)), // +Y
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), // -Y
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)), // +Z
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f)) // -Z
    };
    std::shared_ptr<graphics::Texture> m_equirectangularMap; ///< equirectangular map texture
    unsigned int m_envCubemap = 0; ///< 环境立方体贴图
    unsigned int m_irradianceMap = 0;
    unsigned int m_prefilterMap = 0;
    unsigned int m_brdfLUT = 0;
    unsigned int m_captureFBO = 0, m_captureRBO = 0;
    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;
};

} // namespace pipeline
