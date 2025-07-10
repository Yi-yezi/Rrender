#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "core/Window.h"
#include "resource/ResourceManager.h"
#include "graphics/Camera.h"
#include "graphics/CameraController.h"
#include "graphics/Model.h"
#include "core/InputManager.h"
#include "utils/Time.h"

#include "pipeline/BlinnPhongPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "graphics/Light.h"

using namespace core;
using namespace graphics;
using namespace scene;
using namespace pipeline;

int main() {
    try {
        // 创建窗口
        auto windowPtr = std::make_shared<Window>(800, 600, "Rrender Engine - BlinnPhong");

        // 初始化 GLAD（必须在创建上下文之后）
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glEnable(GL_DEPTH_TEST);

        // 初始化输入与相机控制器
        InputManager::Init(windowPtr);
        auto cameraPtr = std::make_shared<Camera>(Camera::ProjectionType::Perspective);
        cameraPtr->SetPosition(glm::vec3(0.0f, 1.5f, 5.0f));
        auto controllerPtr = std::make_shared<CameraController>(cameraPtr);
        InputManager::RegisterListener(controllerPtr);

        // 资源管理器加载 shader 和模型
        auto shader = ResourceManager::LoadShader(
            "E:/Projects/C++ Projects/Rrender/shaders/blinn_phong/blinnphong.vert",
            "E:/Projects/C++ Projects/Rrender/shaders/blinn_phong/blinnphong.frag");

        auto model = ResourceManager::LoadModel(
            "E:/Projects/C++ Projects/Rrender/assets/objects/backpack/backpack.obj");

        // 创建渲染管线
        BlinnPhongPipeline pipeline(shader);

        // 创建场景并添加实体
        auto scenePtr = std::make_shared<Scene>();

        auto entityPtr = std::make_shared<Entity>(model);
        entityPtr->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        entityPtr->SetScale(glm::vec3(1.0f));
        scenePtr->AddEntity(entityPtr);

        // 添加方向光
        auto dirLight = std::make_shared<DirectionalLight>();
        dirLight->SetDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
        dirLight->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
        dirLight->SetIntensity(0.8f);
        scenePtr->AddLight(dirLight);

        // 添加点光源
        auto pointLight = std::make_shared<PointLight>();
        pointLight->SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
        pointLight->SetColor(glm::vec3(1.0f, 0.5f, 0.5f));
        pointLight->SetIntensity(1.0f);
        pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
        scenePtr->AddLight(pointLight);

        // 添加聚光灯
        auto spotLight = std::make_shared<SpotLight>();
        spotLight->SetPosition(cameraPtr->GetPosition());       // 跟随相机位置
        spotLight->SetDirection(glm::vec3(0.0f, 0.0f, -1.0f)); // 指向前方
        spotLight->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
        spotLight->SetIntensity(1.5f);
        spotLight->SetCutOff(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)));
        spotLight->SetAttenuation(1.0f, 0.09f, 0.032f);
        scenePtr->AddLight(spotLight);

        // 主循环
        while (!windowPtr->ShouldClose()) {
            windowPtr->PollEvents();
            utils::Time::Update(glfwGetTime());
            InputManager::Update();

            // 聚光灯跟随相机（可选）
            spotLight->SetPosition(cameraPtr->GetPosition());
            // 注意这里方向需根据相机前向更新，简单示例：
            glm::vec3 forward = glm::normalize(cameraPtr->GetFront());
            spotLight->SetDirection(forward);

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 使用 BlinnPhong 管线渲染
            pipeline.Render(scenePtr, cameraPtr);

            windowPtr->SwapBuffers();
        }

    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
