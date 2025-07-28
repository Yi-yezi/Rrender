#include "ui/UIManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <algorithm> // for std::clamp

#include "graphics/Light.h"
#include "scene/Entity.h"
#include "resource/ResourceManager.h"
#include "core/InputManager.h"

namespace ui {

bool UIManager::s_Initialized = false;

void UIManager::Init(std::weak_ptr<core::Window> windowPtr, const char* glslVersion) {
    if (s_Initialized) return;

    if (auto win = windowPtr.lock()) {
        GLFWwindow* glfwWindow = win->GetNativeHandle();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init(glslVersion);
        s_Initialized = true;
    }
}

void UIManager::Shutdown() {
    if (!s_Initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    s_Initialized = false;
}

void UIManager::BeginFrame() {
    if (!s_Initialized) return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::RenderUI(const std::shared_ptr<graphics::Camera>& camera,
                         const std::shared_ptr<scene::Scene>& scene,
                         const std::shared_ptr<core::Window>& windowPtr,
                         const std::shared_ptr<pipeline::BRDFPipeline>& brdfPipeline) {
    
    // 获取窗口计算好的UI宽度
    float uiPanelWidth = windowPtr->GetUIWidth();
    float renderWidth = windowPtr->GetRenderWidth();  
    float renderHeight = windowPtr->GetRenderHeight();
    bool compactMode = windowPtr->IsCompactMode();
    
    // 检查是否是相机模式
    bool isCameraMode = core::InputManager::IsCameraMode();
    
    // 设置UI面板位置和大小（固定在右侧）
    ImGui::SetNextWindowPos(ImVec2(renderWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(uiPanelWidth, renderHeight), ImGuiCond_Always);
    
    // 根据模式设置窗口标志
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoTitleBar;
    
    // 相机模式下禁用所有UI交互
    if (isCameraMode) {
        windowFlags |= ImGuiWindowFlags_NoInputs;
    }

    ImGui::Begin("Scene Control", nullptr, windowFlags);
    
    // 显示当前模式状态
    if (isCameraMode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
        ImGui::Text("CAMERA MODE - Press TAB for UI");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
        ImGui::Text("UI MODE - Press TAB for Camera");
        ImGui::PopStyleColor();
    }
    ImGui::Separator();

    // 投影模式 - 使用折叠标题来节省空间
    if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int projectionType = static_cast<int>(camera->GetProjectionType());
        const char* projOptions[] = { "Perspective", "Orthographic" };
        if (ImGui::Combo("Projection", &projectionType, projOptions, IM_ARRAYSIZE(projOptions))) {
            camera->setProjectionType(static_cast<graphics::Camera::ProjectionType>(projectionType));
        }

        // 相机信息 - 根据紧凑模式调整显示格式
        const glm::vec3& pos = camera->GetPosition();
        auto [front, up, right] = camera->GetDirectionVectors();
        
        if (compactMode) {
            ImGui::Text("Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
            ImGui::Text("Dir: %.1f, %.1f, %.1f", front.x, front.y, front.z);
        } else {
            ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
            ImGui::Text("Forward:  %.2f, %.2f, %.2f", front.x, front.y, front.z);
        }

        // 相机控制参数
        if (ImGui::TreeNode("Camera Controls")) {
            static float fov = 45.0f;
            static float nearPlane = 0.1f;
            static float farPlane = 100.0f;
            static float moveSpeed = 2.5f;
            static float sensitivity = 0.1f;

            if (ImGui::SliderFloat("FOV", &fov, 1.0f, 120.0f)) {
                camera->SetFOV(fov);
            }
            if (ImGui::SliderFloat("Near", &nearPlane, 0.001f, 10.0f)) {
                camera->SetNearFar(nearPlane, farPlane);
            }
            if (ImGui::SliderFloat("Far", &farPlane, 1.0f, 1000.0f)) {
                camera->SetNearFar(nearPlane, farPlane);
            }
            if (ImGui::SliderFloat("Speed", &moveSpeed, 0.1f, 10.0f)) {
                camera->SetMovementSpeed(moveSpeed);
            }
            if (ImGui::SliderFloat("Sensitivity", &sensitivity, 0.01f, 1.0f)) {
                camera->SetMouseSensitivity(sensitivity);
            }

            ImGui::TreePop();
        }
    }

    // Pipeline 控制
    if (brdfPipeline && ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 从pipeline获取当前状态
        bool enableIBL = brdfPipeline->IsIBLEnabled();
        bool enableShadow = brdfPipeline->IsShadowEnabled();
        
        // 根据紧凑模式调整布局
        if (compactMode) {
            if (ImGui::Checkbox("IBL", &enableIBL)) {
                brdfPipeline->SetIBL(enableIBL);
            }
            if (ImGui::Checkbox("Shadow", &enableShadow)) {
                brdfPipeline->SetShadow(enableShadow);
            }
        } else {
            if (ImGui::Checkbox("Enable IBL", &enableIBL)) {
                brdfPipeline->SetIBL(enableIBL);
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Enable Shadow", &enableShadow)) {
                brdfPipeline->SetShadow(enableShadow);
            }
        }
        
        // 显示当前状态
        ImGui::Text("IBL: %s | Shadow: %s", 
                   enableIBL ? "ON" : "OFF", 
                   enableShadow ? "ON" : "OFF");
        
        // 显示渲染视口信息
        ImGui::Text("Viewport: %.0fx%.0f", renderWidth, renderHeight);
        if (!compactMode) {
            ImGui::Text("UI Width: %.0fpx", uiPanelWidth);
            ImGui::Text("Aspect: %.2f", windowPtr->GetAspectRatio());
        }
    }

    // 光源控制
    if (ImGui::CollapsingHeader("Lights")) {
        auto& lights = scene->GetLights();
        
        for (size_t i = 0; i < lights.size(); ++i) {
            auto& light = lights[i];
            
            // 根据光源类型显示不同的标签
            std::string label;
            switch (light->GetType()) {
                case graphics::LightType::Directional:
                    label = compactMode ? ("Dir " + std::to_string(i)) : ("Directional Light " + std::to_string(i));
                    break;
                case graphics::LightType::Point:
                    label = compactMode ? ("Point " + std::to_string(i)) : ("Point Light " + std::to_string(i));
                    break;
                case graphics::LightType::Spot:
                    label = compactMode ? ("Spot " + std::to_string(i)) : ("Spot Light " + std::to_string(i));
                    break;
                default:
                    label = "Unknown Light " + std::to_string(i);
                    break;
            }
            
            // 每个光源条目也默认收起
            if (ImGui::TreeNodeEx(label.c_str())) {  // 移除DefaultOpen标志
                // 光源开关
                bool enabled = light->IsEnabled();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    light->SetEnabled(enabled);
                }
                
                // 通用属性：颜色和强度
                glm::vec3 color = light->GetColor()/255.0f; // 假设颜色范围是0-255
                float intensity = light->GetIntensity();
                
                if (compactMode) {
                    if (ImGui::ColorEdit3("##Color", &color.x, ImGuiColorEditFlags_NoInputs)) {
                        light->SetColor(color*255.0f); // 恢复到0-255范围
                    }
                    ImGui::SameLine();
                    ImGui::Text("Color");
                } else {
                    if (ImGui::ColorEdit3("Color", &color.x)) {
                        light->SetColor(color*255.0f); // 恢复到0-255范围
                    }
                }
                
                if (ImGui::SliderFloat(compactMode ? "Int" : "Intensity", &intensity, 0.0f, 10.0f)) {
                    light->SetIntensity(intensity);
                }
                
                // 根据光源类型显示特定属性
                switch (light->GetType()) {
                    case graphics::LightType::Directional: {
                        auto dirLight = std::dynamic_pointer_cast<graphics::DirectionalLight>(light);
                        
                        // 方向光只有方向，没有位置
                        glm::vec3 direction = dirLight->GetDirection();
                        float dragSpeed = compactMode ? 0.01f : 0.05f;
                        if (ImGui::DragFloat3(compactMode ? "Dir" : "Direction", &direction.x, dragSpeed, -1.0f, 1.0f)) {
                            dirLight->SetDirection(glm::normalize(direction));
                        }
                        
                        if (!compactMode) {
                            ImGui::Text("Type: Directional Light");
                            ImGui::Text("Affects: Entire scene");
                        }
                        break;
                    }
                    
                    case graphics::LightType::Point: {
                        auto pointLight = std::dynamic_pointer_cast<graphics::PointLight>(light);
                        
                        // 点光源有位置和衰减参数
                        glm::vec3 position = pointLight->GetPosition();
                        if (ImGui::DragFloat3(compactMode ? "Pos" : "Position", &position.x, 0.1f)) {
                            pointLight->SetPosition(position);
                        }
                        
                        // 衰减参数 - 也默认收起
                        if (ImGui::TreeNode(compactMode ? "Atten" : "Attenuation")) {
                            float constant = pointLight->GetConstant();
                            float linear = pointLight->GetLinear();
                            float quadratic = pointLight->GetQuadratic();
                            
                            if (ImGui::SliderFloat("Constant", &constant, 0.0f, 2.0f)) {
                                pointLight->SetAttenuation(constant, linear, quadratic);
                            }
                            if (ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f)) {
                                pointLight->SetAttenuation(constant, linear, quadratic);
                            }
                            if (ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f)) {
                                pointLight->SetAttenuation(constant, linear, quadratic);
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        if (!compactMode) {
                            ImGui::Text("Type: Point Light");
                            glm::vec3 pos = pointLight->GetPosition();
                            ImGui::Text("Distance: %.2fm", glm::length(pos));
                        }
                        break;
                    }
                    
                    case graphics::LightType::Spot: {
                        auto spotLight = std::dynamic_pointer_cast<graphics::SpotLight>(light);
                        
                        // 聚光灯有位置、方向和锥角
                        glm::vec3 position = spotLight->GetPosition();
                        glm::vec3 direction = spotLight->GetDirection();
                        
                        if (ImGui::DragFloat3(compactMode ? "Pos" : "Position", &position.x, 0.1f)) {
                            spotLight->SetPosition(position);
                        }
                        
                        float dragSpeed = compactMode ? 0.01f : 0.05f;
                        if (ImGui::DragFloat3(compactMode ? "Dir" : "Direction", &direction.x, dragSpeed, -1.0f, 1.0f)) {
                            spotLight->SetDirection(glm::normalize(direction));
                        }
                        
                        // 聚光灯特有的锥角设置
                        float innerCutOff = spotLight->GetInnerCutOff();
                        float outerCutOff = spotLight->GetOuterCutOff();
                        
                        // 转换为角度显示（更直观）
                        float innerAngle = glm::degrees(glm::acos(innerCutOff));
                        float outerAngle = glm::degrees(glm::acos(outerCutOff));
                        
                        if (compactMode) {
                            if (ImGui::SliderFloat("Inner", &innerAngle, 0.0f, outerAngle)) {
                                spotLight->SetCutOff(glm::cos(glm::radians(innerAngle)), outerCutOff);
                            }
                            if (ImGui::SliderFloat("Outer", &outerAngle, innerAngle, 90.0f)) {
                                spotLight->SetCutOff(innerCutOff, glm::cos(glm::radians(outerAngle)));
                            }
                        } else {
                            if (ImGui::SliderFloat("Inner Angle", &innerAngle, 0.0f, outerAngle, "%.1f°")) {
                                spotLight->SetCutOff(glm::cos(glm::radians(innerAngle)), outerCutOff);
                            }
                            if (ImGui::SliderFloat("Outer Angle", &outerAngle, innerAngle, 90.0f, "%.1f°")) {
                                spotLight->SetCutOff(innerCutOff, glm::cos(glm::radians(outerAngle)));
                            }
                        }
                        
                        // 衰减参数（继承自点光源）- 也默认收起
                        if (ImGui::TreeNode(compactMode ? "Atten" : "Attenuation")) {
                            float constant = spotLight->GetConstant();
                            float linear = spotLight->GetLinear();
                            float quadratic = spotLight->GetQuadratic();
                            
                            if (ImGui::SliderFloat("Constant", &constant, 0.0f, 2.0f)) {
                                spotLight->SetAttenuation(constant, linear, quadratic);
                            }
                            if (ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f)) {
                                spotLight->SetAttenuation(constant, linear, quadratic);
                            }
                            if (ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f)) {
                                spotLight->SetAttenuation(constant, linear, quadratic);
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        if (!compactMode) {
                            ImGui::Text("Type: Spot Light");
                            ImGui::Text("Cone: %.1f° - %.1f°", innerAngle, outerAngle);
                            glm::vec3 pos = spotLight->GetPosition();
                            ImGui::Text("Distance: %.2fm", glm::length(pos));
                        }
                        break;
                    }
                    
                    default:
                        ImGui::Text("Unknown light type");
                        break;
                }
                
                ImGui::TreePop();
            }
        }
        
        // 添加新光源的按钮
        ImGui::Separator();
        if (ImGui::Button(compactMode ? "+" : "Add Light")) {
            ImGui::OpenPopup("LightTypePopup");
        }
        
        if (ImGui::BeginPopup("LightTypePopup")) {
            if (ImGui::MenuItem("Directional Light")) {
                auto newLight = std::make_shared<graphics::DirectionalLight>();
                newLight->SetDirection(glm::vec3(0.0f, -1.0f, 0.0f)); // 默认向下
                newLight->SetColor(glm::vec3(255.0f)); // 使用255制
                newLight->SetIntensity(1.0f);
                scene->AddLight(newLight);
            }
            if (ImGui::MenuItem("Point Light")) {
                auto newLight = std::make_shared<graphics::PointLight>();
                newLight->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f)); // 默认在上方
                newLight->SetColor(glm::vec3(255.0f)); // 使用255制
                newLight->SetIntensity(1.0f);
                scene->AddLight(newLight);
            }
            if (ImGui::MenuItem("Spot Light")) {
                auto newLight = std::make_shared<graphics::SpotLight>();
                newLight->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
                newLight->SetDirection(glm::vec3(0.0f, -1.0f, 0.0f)); // 默认向下照射
                newLight->SetColor(glm::vec3(255.0f)); // 使用255制
                newLight->SetIntensity(1.0f);
                newLight->SetCutOff(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));
                scene->AddLight(newLight);
            }
            ImGui::EndPopup();
        }
        
        if (lights.empty()) {
            ImGui::TextDisabled("No lights in scene");
            ImGui::Text("Click 'Add Light' to create one");
        } else {
            // 显示光源统计
            if (!compactMode) {
                int dirCount = 0, pointCount = 0, spotCount = 0;
                for (auto& light : lights) {
                    if (!light->IsEnabled()) continue;
                    switch (light->GetType()) {
                        case graphics::LightType::Directional: dirCount++; break;
                        case graphics::LightType::Point: pointCount++; break;
                        case graphics::LightType::Spot: spotCount++; break;
                    }
                }
                ImGui::Text("Active: Dir(%d) Point(%d) Spot(%d)", dirCount, pointCount, spotCount);
            }
        }
    }

    // 场景物体控制
    if (ImGui::CollapsingHeader("Scene Objects")) {
        auto& objects = scene->GetEntities();
        
        for (size_t i = 0; i < objects.size(); ++i) {
            auto& obj = objects[i];
            std::string label = "Object " + std::to_string(i);
            
            if (ImGui::TreeNodeEx(label.c_str())) {
                // 物体变换
                glm::vec3 position = obj->GetPosition();
                glm::vec3 rotation = obj->GetRotation();
                glm::vec3 scale = obj->GetScale();
                
                if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
                    obj->SetPosition(position);
                }
                if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f, -180.0f, 180.0f)) {
                    obj->SetRotation(rotation);
                }
                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 5.0f)) {
                    obj->SetScale(scale);
                }
                
                // 可见性
                bool visible = obj->IsVisible();
                if (ImGui::Checkbox("Visible", &visible)) {
                    obj->SetVisible(visible);
                }
                
                ImGui::TreePop();
            }
        }
        
        if (objects.empty()) {
            ImGui::TextDisabled("No objects in scene");
        }
    }

    // 调试信息 - 默认展开
    if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 相机信息（无论哪种模式都显示）
        const glm::vec3& pos = camera->GetPosition();
        auto [front, up, right] = camera->GetDirectionVectors();
        
        ImGui::Text("Camera Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
        ImGui::Text("Camera Direction: %.2f, %.2f, %.2f", front.x, front.y, front.z);
        ImGui::Separator();
        
        // 场景统计信息
        auto& lights = scene->GetLights();
        int activeCount = 0;
        for (auto& light : lights) {
            if (light->IsEnabled()) activeCount++;
        }
        ImGui::Text("Active Lights: %d / %zu", activeCount, lights.size());
        
        auto& objects = scene->GetEntities();
        ImGui::Text("Scene Objects: %zu", objects.size());
        ImGui::Separator();
        
        // 窗口和渲染信息
        auto [windowWidth, windowHeight] = windowPtr->GetSize();
        ImGui::Text("Window: %dx%d", windowWidth, windowHeight);
        ImGui::Text("Viewport: %.0fx%.0f", renderWidth, renderHeight);
        ImGui::Text("UI Width: %.0fpx", uiPanelWidth);
        ImGui::Text("Aspect Ratio: %.2f", windowPtr->GetAspectRatio());
        ImGui::Text("Compact Mode: %s", compactMode ? "ON" : "OFF");
        ImGui::Separator();
        
        // 性能信息
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        
        // 输入模式状态
        ImGui::Separator();
        ImGui::Text("Input Mode: %s", isCameraMode ? "Camera Control" : "UI Control");
        if (isCameraMode) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Press TAB to enable UI");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "Press TAB for camera control");
        }
    }

    ImGui::End();
}

void UIManager::EndFrame() {
    if (!s_Initialized) return;
    
    // 在渲染后设置输入捕获标志
    ImGuiIO& io = ImGui::GetIO();
    bool isCameraMode = core::InputManager::IsCameraMode();
    
    if (isCameraMode) {
        // 相机模式：强制禁用UI输入捕获
        io.WantCaptureKeyboard = false;
        io.WantCaptureMouse = false;
        io.WantTextInput = false;
    }
    // UI模式下让ImGui自己决定是否捕获输入
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace ui
