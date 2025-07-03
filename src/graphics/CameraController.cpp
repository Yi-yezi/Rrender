#include "graphics/CameraController.h"

namespace graphics{
    CameraController::CameraController()
        : m_Mode(CameraMode::Arcball), m_Position(0.0f, 0.0f, 3.0f), m_Front(0.0f, 0.0f, -1.0f),
          m_Up(0.0f, 1.0f, 0.0f), m_WorldUp(0.0f, 1.0f, 0.0f), m_Yaw(-90.0f), m_Pitch(0.0f),
          m_MoveSpeed(2.5f), m_MouseSensitivity(0.1f), m_Target(0.0f, 0.0f, 0.0f),
          m_DistanceToTarget(5.0f), m_ArcballYaw(90.0f), m_ArcballPitch(45.0f) {
        // 初始化相机向量
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    }

    void CameraController::SetMode(CameraMode mode) {
        m_Mode = mode;
    }

    CameraController::CameraMode CameraController::GetMode() const {
        return m_Mode;
    }

    glm::mat4 CameraController::GetViewMatrix() const {
        if (m_Mode == CameraMode::FirstPerson) {
            return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
        } else { // Arcball mode
            glm::vec3 position = m_Target + glm::vec3(
                m_DistanceToTarget * cos(glm::radians(m_ArcballYaw)) * cos(glm::radians(m_ArcballPitch)),
                m_DistanceToTarget * sin(glm::radians(m_ArcballPitch)),
                -m_DistanceToTarget * sin(glm::radians(m_ArcballYaw)) * cos(glm::radians(m_ArcballPitch))
            );
            return glm::lookAt(position, m_Target, m_Up);
        }
    }

    glm::vec3 CameraController::GetPosition() const {
        return (m_Mode == CameraMode::FirstPerson) ? m_Position : (m_Target + glm::vec3(
            m_DistanceToTarget * cos(glm::radians(m_ArcballYaw)) * cos(glm::radians(m_Arc
}
