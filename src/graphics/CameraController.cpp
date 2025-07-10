#include "graphics/CameraController.h"
#include "core/InputManager.h"
#include "utils/Time.h"


namespace graphics {

    CameraController::CameraController(std::shared_ptr<Camera> camera)
        : m_Camera(camera),
          m_MoveSpeed(3.0f),
          m_MouseSensitivity(0.1f),
          m_ConstrainPitch(true) {}

    void CameraController::SetCamera(std::shared_ptr<Camera>& camera) {
        m_Camera = std::move(camera);
    }

    void CameraController::SetMoveSpeed(float speed) {
        m_MoveSpeed = speed;
    }

    void CameraController::SetMouseSensitivity(float sensitivity) {
        m_MouseSensitivity = sensitivity;
    }

    void CameraController::SetPitchConstraint(bool enabled) {
        m_ConstrainPitch = enabled;
    }

    void CameraController::OnInput() {
        if (!m_Camera) return;

        float deltaTime = utils::Time::GetDeltaTime();
        std::tuple<glm::vec3, glm::vec3, glm::vec3> directions = m_Camera->GetDirectionVectors(); // 获取前、上、右向量

        // 键盘移动
        
        if (core::InputManager::IsKeyDown(GLFW_KEY_W)) {
            m_Camera->SetPosition(m_Camera->GetPosition() + std::get<0>(directions) * m_MoveSpeed * deltaTime);
        }
        if (core::InputManager::IsKeyDown(GLFW_KEY_S)) {
            m_Camera->SetPosition(m_Camera->GetPosition() - std::get<0>(directions) * m_MoveSpeed * deltaTime);
        }
        if (core::InputManager::IsKeyDown(GLFW_KEY_A)) {
            m_Camera->SetPosition(m_Camera->GetPosition() - std::get<2>(directions) * m_MoveSpeed * deltaTime);
        }
        if (core::InputManager::IsKeyDown(GLFW_KEY_D)) {
            m_Camera->SetPosition(m_Camera->GetPosition() + std::get<2>(directions) * m_MoveSpeed * deltaTime);
        }
        if (core::InputManager::IsKeyDown(GLFW_KEY_SPACE)) {
            m_Camera->SetPosition(m_Camera->GetPosition() + std::get<1>(directions) * m_MoveSpeed * deltaTime);
        }
        if (core::InputManager::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
            m_Camera->SetPosition(m_Camera->GetPosition() - std::get<1>(directions) * m_MoveSpeed * deltaTime);
        }

        // 鼠标控制旋转
        auto [mouseDeltaX, mouseDeltaY] = core::InputManager::GetMouseDelta();
        m_Camera->AddRotation(static_cast<float>(mouseDeltaX) * m_MouseSensitivity,
                            static_cast<float>(mouseDeltaY)* m_MouseSensitivity,
                              m_ConstrainPitch);
    }

}
