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
    #include "pipeline/ShadowMappingPipeline.h"
    #include "pipeline/PointShadowMappingPipeline.h"
    #include "pipeline/DeferredShadingPipeline.h"
    #include "pipeline/HDRPipeline.h"
    #include "pipeline/PBRPipeline.h"
    #include "scene/Scene.h"
    #include "scene/Entity.h"
    #include "graphics/Light.h"
    #include "utils/PathResolver.h"
    #include "ui/UIManager.h"
    #include "assets/VerticesData.h"
    #include "utils/utils.h"

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
            //auto shader = ResourceManager::LoadShader(
            //    PathResolver::Resolve("shaders/blinn_phong/blinnphong.vert"),
            //    PathResolver::Resolve("shaders/blinn_phong/blinnphong.frag"));
            //auto model = ResourceManager::LoadModel(
            //    PathResolver::Resolve("assets/objects/backpack/backpack.obj"));

            auto mainShader = ResourceManager::LoadShader(
                PathResolver::Resolve("shaders/pbr/pbr.vert"),
                PathResolver::Resolve("shaders/pbr/pbr.frag"));

            auto geometryShader = ResourceManager::LoadShader(
                PathResolver::Resolve("shaders/deferred/geometry_pass.vert"),
                PathResolver::Resolve("shaders/deferred/geometry_pass.frag"));

            auto hdrShader = ResourceManager::LoadShader(
                PathResolver::Resolve("shaders/hdr/hdr.vert"),
                PathResolver::Resolve("shaders/hdr/hdr.frag"));

            auto debugShader = ResourceManager::LoadShader(
                PathResolver::Resolve("shaders/shadow_mapping/debug.vert"),
                PathResolver::Resolve("shaders/shadow_mapping/debug.frag"));

            

            





            // 使用自定义顶点数据创建模型
            auto cubeModel = ResourceManager::LoadModel(
                "cube",
                ParseVertexData(cubeVertices,36), // 使用自定义立方体顶点数据
                PathResolver::Resolve("assets/textures/wood.png"),
                false);

            auto planeModel = ResourceManager::LoadModel(
                "plane",
                ParseVertexData(planeVertices, 6), // 使用自定义平面顶点数据
                PathResolver::Resolve("assets/textures/wood.png"),
                false);

            auto backpackModel = ResourceManager::LoadModel(
                PathResolver::Resolve("assets/objects/backpack/backpack.obj"));


            // 创建渲染管线
            //BlinnPhongPipeline pipeline(shader);
            //OutlinePipeline pipeline(
            //    shader,
            //    ResourceManager::LoadShader(
            //        PathResolver::Resolve("shaders/outline/outline.vert"),
            //        PathResolver::Resolve("shaders/outline/outline.frag")));
            //ShadowMappingPipeline pipeline(depthShader, mainShader);
            //PointShadowMappingPipeline pipeline(depthShader, mainShader);
            //HDRPipeline pipeline(hdrShader,mainShader, windowPtr);
            //DeferredShadingPipeline pipeline(geometryShader, mainShader, windowPtr);
            PBRPipeline pipeline(mainShader, windowPtr);


            // 创建场景和实体
            auto scenePtr = std::make_shared<Scene>();
            auto entityPtr = std::make_shared<Entity>(backpackModel);
            entityPtr->SetPosition(glm::vec3(-1.0f,1.0f,0.2f));
            entityPtr->SetScale(glm::vec3(0.2f));
            scenePtr->AddEntity(entityPtr);

            //auto entity = std::make_shared<Entity>(cubeModel);
            //entity->SetPosition(glm::vec3(0.0f));
            //entity->SetScale(glm::vec3(1.0f));
            //scenePtr->AddEntity(entity);
            //添加多个实体

            auto entity = std::make_shared<Entity>(cubeModel);
            entity->SetPosition(glm::vec3(0.0f, 1.5f, 0.0));
            entity->SetScale(glm::vec3(0.5f));
            scenePtr->AddEntity(entity);
            entity = std::make_shared<Entity>(cubeModel);
            entity->SetPosition(glm::vec3(2.0f, 0.0f, 1.0));
            entity->SetScale(glm::vec3(0.5f));
            scenePtr->AddEntity(entity);
            entity = std::make_shared<Entity>(cubeModel);
            entity->SetPosition(glm::vec3(-1.0f, 0.0f, 2.0));
            entity->SetRotation(glm::vec3(0.0f, 60.0f, 0.0f));
            entity->SetScale(glm::vec3(0.25f));
            scenePtr->AddEntity(entity);


            //添加平面实体
            auto planeEntity = std::make_shared<Entity>(planeModel);
            planeEntity->SetPosition(glm::vec3(0.0f));
            scenePtr->AddEntity(planeEntity);

            

            // 添加方向光
            auto dirLight = std::make_shared<DirectionalLight>();
            dirLight->SetDirection(glm::vec3(2.0f, -4.0f, 1.0f));
            dirLight->SetColor(glm::vec3(1.0f));
            dirLight->SetIntensity(0.8f);
            //scenePtr->AddLight(dirLight);

            // 添加点光源
            auto pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3(2.0f, 4.0f, 1.0f));
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
            //scenePtr->AddLight(spotLight);

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
