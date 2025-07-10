#include "ui/UIManager.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include "graphics/Light.h"
#include "scene/Entity.h"
#include "resource/ResourceManager.h"

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

void UIManager::EndFrame() {
    if (!s_Initialized) return;
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::RenderUI(const std::shared_ptr<graphics::Camera>& camera,
                         const std::shared_ptr<scene::Scene>& scene) {
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene Control");

    // 投影模式
    static int projectionType = static_cast<int>(camera->GetProjectionType());
    const char* projOptions[] = { "Perspective", "Orthographic" };
    if (ImGui::Combo("Projection", &projectionType, projOptions, IM_ARRAYSIZE(projOptions))) {
        camera->setProjectionType(static_cast<graphics::Camera::ProjectionType>(projectionType));
    }

    // 相机信息
    const glm::vec3& pos = camera->GetPosition();
    auto [front, up, right] = camera->GetDirectionVectors();
    ImGui::Text("Camera Pos: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
    ImGui::Text("Camera Front: %.2f %.2f %.2f", front.x, front.y, front.z);

    // 光源
    auto& lights = scene->GetLights();
    for (size_t i = 0; i < lights.size(); ++i) {
        auto& light = lights[i];
        std::string label = "Light " + std::to_string(i);
        if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            // 显示光源类型
            const char* types[] = { "Directional", "Point", "Spot" };
            ImGui::Text("Type: %s", types[static_cast<int>(light->GetType())]);

            // Direction 可编辑
            glm::vec3 dir = light->GetDirection();
            if (ImGui::DragFloat3("Direction", &dir.x, 0.1f)) {
                light->SetDirection(dir);
            }

            // 颜色
            glm::vec3 color = light->GetColor();
            if (ImGui::ColorEdit3("Color", &color.x)) {
                light->SetColor(color);
            }

            // 强度
            float intensity = light->GetIntensity();
            if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f)) {
                light->SetIntensity(intensity);
            }

            // Point/Spot 额外参数
            if (light->GetType() != graphics::Light::Type::Directional) {
                glm::vec3 pos = light->GetPosition();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                    light->SetPosition(pos);
                }

                float c = light->GetConstant();
                float l = light->GetLinear();
                float q = light->GetQuadratic();
                ImGui::SliderFloat("Constant", &c, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", &l, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", &q, 0.0f, 2.0f);
                light->SetAttenuation(c, l, q);
            }

            if (light->GetType() == graphics::Light::Type::Spot) {
                float inner = light->GetInnerCutOff();
                float outer = light->GetOuterCutOff();
                ImGui::SliderFloat("Inner Cutoff", &inner, 0.0f, 1.0f);
                ImGui::SliderFloat("Outer Cutoff", &outer, 0.0f, 1.0f);
                light->SetCutOff(inner, outer);
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();
}

} // namespace ui
