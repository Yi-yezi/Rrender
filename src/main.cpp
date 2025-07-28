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
    #include "pipeline/IBLPipeline.h"
    #include "pipeline/BRDFPipeline.h"
    #include "renderPass/ShadowPass.h"
    #include "pipeline/BRDFPipeline.h"
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
    using namespace utils;

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
            auto controllerPtr = std::make_shared<CameraController>(cameraPtr);
            InputManager::RegisterListener(controllerPtr);


            



            
            // 使用自定义顶点数据创建模型
            //std::vector<Vertex> vertices;
            //std::vector<unsigned int> indices;
            //utils::GenerateSphere(vertices, indices);
            //auto ironSphereModel = std::make_shared<Model>(vertices, indices,std::vector<std::string>{
            //    PathResolver::Resolve("assets/textures/pbr/rusted_iron/albedo.png"),
            //    PathResolver::Resolve("assets/textures/pbr/rusted_iron/normal.png"),
            //    PathResolver::Resolve("assets/textures/pbr/rusted_iron/metallic.png"),
            //    PathResolver::Resolve("assets/textures/pbr/rusted_iron/roughness.png"),
            //    PathResolver::Resolve("assets/textures/pbr/rusted_iron/ao.png")
            //}, false, GL_TRIANGLE_STRIP);
//
            //auto goldSphereModel = std::make_shared<Model>(vertices, indices,std::vector<std::string>{
            //    PathResolver::Resolve("assets/textures/pbr/gold/albedo.png"),
            //    PathResolver::Resolve("assets/textures/pbr/gold/normal.png"),
            //    PathResolver::Resolve("assets/textures/pbr/gold/metallic.png"),
            //    PathResolver::Resolve("assets/textures/pbr/gold/roughness.png"),
            //    PathResolver::Resolve("assets/textures/pbr/gold/ao.png")
            //}, false, GL_TRIANGLE_STRIP);
//
            //auto grassSphereModel = std::make_shared<Model>(vertices, indices,std::vector<std::string>{
            //    PathResolver::Resolve("assets/textures/pbr/grass/albedo.png"),
            //    PathResolver::Resolve("assets/textures/pbr/grass/normal.png"),
            //    PathResolver::Resolve("assets/textures/pbr/grass/metallic.png"),
            //    PathResolver::Resolve("assets/textures/pbr/grass/roughness.png"),
            //    PathResolver::Resolve("assets/textures/pbr/grass/ao.png")
            //}, false, GL_TRIANGLE_STRIP);
//
            //auto plasticSphereModel = std::make_shared<Model>(vertices, indices,std::vector<std::string>{
            //    PathResolver::Resolve("assets/textures/pbr/plastic/albedo.png"),
            //    PathResolver::Resolve("assets/textures/pbr/plastic/normal.png"),
            //    PathResolver::Resolve("assets/textures/pbr/plastic/metallic.png"),
            //    PathResolver::Resolve("assets/textures/pbr/plastic/roughness.png"),
            //    PathResolver::Resolve("assets/textures/pbr/plastic/ao.png")
            //}, false, GL_TRIANGLE_STRIP);
//
            //auto wallSphereModel = std::make_shared<Model>(vertices, indices,std::vector<std::string>{
            //    PathResolver::Resolve("assets/textures/pbr/wall/albedo.png"),
            //    PathResolver::Resolve("assets/textures/pbr/wall/normal.png"),
            //    PathResolver::Resolve("assets/textures/pbr/wall/metallic.png"),
            //    PathResolver::Resolve("assets/textures/pbr/wall/roughness.png"),
            //    PathResolver::Resolve("assets/textures/pbr/wall/ao.png")
            //}, false, GL_TRIANGLE_STRIP);

            


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
            //IBLPipeline pipeline(
            //    pbrShader,
            //    irradianceShader,
            //    prefilterShader,
            //    brdfShader,
            //    backgroundShader,
            //    equirectangularMap2cubemapShader,
            //    PathResolver::Resolve("assets/textures/hdr/newport_loft.hdr"),
            //    windowPtr);
            BRDFPipeline pipeline(windowPtr);


            // 创建场景和实体
            auto scenePtr = std::make_shared<Scene>();
            auto robotModel = ResourceManager::LoadModel(PathResolver::Resolve("assets/objects/behemoth_from_horizon_zero_dawn.glb"));
            auto entity = std::make_shared<Entity>(robotModel);
            entity->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            entity->SetScale(glm::vec3(0.5f));
            scenePtr->AddEntity(entity);
            entity = std::make_shared<Entity>(robotModel);
            entity->SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));
            entity->SetScale(glm::vec3(0.5f));
            scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(robotModel);
            //entity->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));
            //entity->SetScale(glm::vec3(0.5f));
            //scenePtr->AddEntity(entity);
            //auto entity = std::make_shared<Entity>(ironSphereModel);
            //entity->SetPosition(glm::vec3(-5.0, 0.0, 2.0));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(goldSphereModel);
            //entity->SetPosition(glm::vec3(-3.0, 0.0, 2.0));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(grassSphereModel);
            //entity->SetPosition(glm::vec3(-1.0, 0.0, 2.0));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(plasticSphereModel);
            //entity->SetPosition(glm::vec3(1.0, 0.0, 2.0));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(wallSphereModel);
            //entity->SetPosition(glm::vec3(3.0, 0.0, 2.0));
            //scenePtr->AddEntity(entity);
            //auto entityPtr = std::make_shared<Entity>(backpackModel);
            //entityPtr->SetPosition(glm::vec3(-1.0f,1.0f,0.2f));
            //entityPtr->SetScale(glm::vec3(0.2f));
            //scenePtr->AddEntity(entityPtr);

            //auto entity = std::make_shared<Entity>(cubeModel);
            //entity->SetPosition(glm::vec3(0.0f));
            //entity->SetScale(glm::vec3(1.0f));
            //scenePtr->AddEntity(entity);
            //添加多个实体

            //auto entity = std::make_shared<Entity>(cubeModel);
            //entity->SetPosition(glm::vec3(0.0f, 1.5f, 0.0));
            //entity->SetScale(glm::vec3(0.5f));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(cubeModel);
            //entity->SetPosition(glm::vec3(2.0f, 0.0f, 1.0));
            //entity->SetScale(glm::vec3(0.5f));
            //scenePtr->AddEntity(entity);
            //entity = std::make_shared<Entity>(cubeModel);
            //entity->SetPosition(glm::vec3(-1.0f, 0.0f, 2.0));
            //entity->SetRotation(glm::vec3(0.0f, 60.0f, 0.0f));
            //entity->SetScale(glm::vec3(0.25f));
            //scenePtr->AddEntity(entity);


            //添加平面实体
            //auto planeEntity = std::make_shared<Entity>(planeModel);
            //planeEntity->SetPosition(glm::vec3(0.0f));
            //scenePtr->AddEntity(planeEntity);

            

            // 添加方向光
            auto dirLight = std::make_shared<DirectionalLight>();
            dirLight->SetDirection(glm::vec3(2.0f, -4.0f, 1.0f));
            dirLight->SetColor(glm::vec3(1.0f));
            dirLight->SetIntensity(0.8f);
            scenePtr->AddLight(dirLight);

            // 添加点光源
            auto pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3(-4.0f,  0.0f, 0.0f));
            pointLight->SetColor(glm::vec3(50.0f));
            pointLight->SetIntensity(1.0f);
            pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(pointLight);

            pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3( 10.0f,  10.0f, 10.0f));
            pointLight->SetColor(glm::vec3(50.0f));
            pointLight->SetIntensity(1.0f);
            pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(pointLight);

            pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3(-10.0f, -10.0f, 10.0f));
            pointLight->SetColor(glm::vec3(50.0f));
            pointLight->SetIntensity(1.0f);
            pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(pointLight);

            pointLight = std::make_shared<PointLight>();
            pointLight->SetPosition(glm::vec3( 10.0f, -10.0f, 10.0f));
            pointLight->SetColor(glm::vec3(50.0f));
            pointLight->SetIntensity(1.0f);
            pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);
            scenePtr->AddLight(pointLight);




            // 添加聚光灯
            auto spotLight = std::make_shared<SpotLight>();
            spotLight->SetPosition(cameraPtr->GetPosition());
            spotLight->SetDirection(glm::normalize(cameraPtr->GetFront()));
            spotLight->SetColor(glm::vec3(200.0f));
            spotLight->SetIntensity(300.0f);
            spotLight->SetCutOff(glm::cos(glm::radians(5.5f)), glm::cos(glm::radians(17.5f)));
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
                pipeline.Render(scenePtr, cameraPtr, true, true);


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
