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
    #include "pipeline/OutlinePipeline.h"
    #include "scene/Scene.h"
    #include "scene/Entity.h"
    #include "graphics/Light.h"
    #include "utils/PathResolver.h"
    #include "ui/UIManager.h"

    using namespace core;
    using namespace graphics;
    using namespace scene;
    using namespace pipeline;
    using namespace ui;

    int main() {
        try {
            // 创建窗口
            auto windowPtr = std::make_shared<Window>(1280, 720, "Rrender Engine - BlinnPhong");

            // 初始化GLAD
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                throw std::runtime_error("Failed to initialize GLAD");
            }
            

            // 初始化输入和相机控制器
            InputManager::Init(windowPtr);
            auto cameraPtr = std::make_shared<Camera>(Camera::ProjectionType::Perspective);
            cameraPtr->SetPosition(glm::vec3(0.0f, 1.5f, 5.0f));
            //int width, height;
            //windowPtr->GetFrameBufferSize(width, height);
            //cameraPtr->SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
            //glViewport(0, 0, width, height);
            auto controllerPtr = std::make_shared<CameraController>(cameraPtr);
            InputManager::RegisterListener(controllerPtr);

            // 载入着色器和模型资源
            auto shader = ResourceManager::LoadShader(
                PathResolver::Resolve("shaders/blinn_phong/blinnphong.vert"),
                PathResolver::Resolve("shaders/blinn_phong/blinnphong.frag"));
            auto model = ResourceManager::LoadModel(
                PathResolver::Resolve("assets/objects/backpack/backpack.obj"));

            // 创建渲染管线
            //BlinnPhongPipeline pipeline(shader);
            OutlinePipeline pipeline(
                shader,
                ResourceManager::LoadShader(
                    PathResolver::Resolve("shaders/outline/outline.vert"),
                    PathResolver::Resolve("shaders/outline/outline.frag")));

            // 创建场景和实体
            auto scenePtr = std::make_shared<Scene>();
            auto entityPtr = std::make_shared<Entity>(model);
            entityPtr->SetPosition(glm::vec3(0.0f));
            entityPtr->SetScale(glm::vec3(1.0f));
            scenePtr->AddEntity(entityPtr);

            // 添加方向光
            auto dirLight = std::make_shared<DirectionalLight>();
            dirLight->SetDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
            dirLight->SetColor(glm::vec3(1.0f));
            dirLight->SetIntensity(0.8f);
            scenePtr->AddLight(dirLight);

            // 添加点光源
            auto pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
            pointLight->SetColor(glm::vec3(1.0f));
            pointLight->SetIntensity(1.0f);
            pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(pointLight);

            // 添加聚光灯
            auto spotLight = std::make_shared<SpotLight>();
            spotLight->SetPosition(cameraPtr->GetPosition());
            spotLight->SetDirection(glm::normalize(cameraPtr->GetFront()));
            spotLight->SetColor(glm::vec3(1.0f));
            spotLight->SetIntensity(1.5f);
            spotLight->SetCutOff(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f)));
            spotLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(spotLight);

            // 初始化ImGui
            UIManager::Init(windowPtr, "#version 330 core");

            // 主循环
            while (!windowPtr->ShouldClose()) {
                windowPtr->PollEvents();
                utils::Time::Update(glfwGetTime());
                InputManager::Update();

                // 聚光灯随相机移动和转向
                spotLight->SetPosition(cameraPtr->GetPosition());
                spotLight->SetDirection(glm::normalize(cameraPtr->GetFront()));

                // 清屏


                // 开始新帧UI绘制
                UIManager::BeginFrame();

                // 渲染场景
                pipeline.Render(scenePtr, cameraPtr);

                // 渲染UI界面，传入相机和场景
                UIManager::RenderUI(cameraPtr, scenePtr);

                // 结束UI绘制，提交绘制命令
                UIManager::EndFrame();

                windowPtr->SwapBuffers();
            }

            // 关闭时清理ImGui
            UIManager::Shutdown();

        } catch (const std::exception& e) {
            std::cerr << "[Error] " << e.what() << std::endl;
            return -1;
        }

        return 0;
    }
