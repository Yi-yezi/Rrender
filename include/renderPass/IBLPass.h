#include "renderPass/RenderPass.h"
#include "scene/Scene.h"
#include "graphics/Shader.h"
#include "core/Window.h"
#include "graphics/Texture.h"
#include <memory>

namespace pipeline {
class IBLPass : public RenderPass {
public:
    IBLPass(std::shared_ptr<core::Window> windowPtr,const std::string& equirectangularMapPath);
    void Execute(const std::shared_ptr<scene::Scene>& scene=nullptr) override;
    void BindMap();
    ~IBLPass() override;
private:
    void RenderEnvironmentMap();
    void RenderIrradianceMap();
    void RenderPrefilterMap();
    void RenderBRDFLUT();
    void RenderCube();
    void RenderQuad();


    std::shared_ptr<core::Window> m_WindowPtr; ///< 窗口
    std::shared_ptr<graphics::Shader> m_IrradianceShader; ///< 辐照度贴图着色器
    std::shared_ptr<graphics::Shader> m_PrefilterShader; ///< 预过滤贴图着色器
    std::shared_ptr<graphics::Shader> m_BRDFShader; ///< BRDF LUT着色器
    std::shared_ptr<graphics::Shader> m_Equirectangular2CubemapShader; ///< Equirectangular到立方体贴图转换着色器

    std::shared_ptr<graphics::Texture> m_EquirectangularMap; ///< Equirectangular贴图
    unsigned int m_EnvCubemap = 0; ///< 环境立方体贴图
    unsigned int m_IrradianceMap = 0; ///< 辐照度贴
    unsigned int m_PrefilterMap = 0; ///< 预过滤贴图
    unsigned int m_BRDFLUT = 0; ///< BRDF LUT贴图
    unsigned int m_CaptureFBO = 0; ///< 捕获FBO
    unsigned int m_CaptureRBO = 0; ///< 捕获RBO
    unsigned int m_CubeVAO = 0; ///< 立方体VAO
    unsigned int m_CubeVBO = 0; ///< 立方体VBO
    unsigned int m_QuadVAO = 0; ///< 四边形VAO
    unsigned int m_QuadVBO = 0; ///< 四边形VBO
    int m_Width = 1280; ///< 窗口宽度
    int m_Height = 720; ///< 窗口高度
    std::string m_EquirectangularMapPath; ///< Equirectangular贴图路径

    const glm::mat4 m_CaptureViews[6] = {
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)), // +X
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)), // -X
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)), // +Y
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), // -Y
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)), // +Z
        glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f)) // -Z
    };

};
}