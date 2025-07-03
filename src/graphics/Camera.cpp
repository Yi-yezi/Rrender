#include "graphics/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

    Camera::Camera(ProjectionType type = ProjectionType::Perspective) : m_ProjectionType(type) {
        UpdateDirection(); // 初始化方向向量
    }
    
    
    glm::mat4 Camera::GetViewMatrix() const {
        // 计算视图矩阵
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }
    
    glm::mat4 Camera::GetProjectionMatrix() const {
		if (m_ProjectionType == ProjectionType::Perspective) {
			return glm::perspective(glm::radians(m_Fov), m_Aspect, m_Near, m_Far);
		}
		else {
			return glm::ortho(m_Left, m_RightBound, m_Bottom, m_Top, m_Near, m_Far);
		}
    }
    
    void Camera::SetPosition(const glm::vec3& pos) {
        m_Position = pos;
    }
    
    void Camera::SetRotation(float yawDegrees, float pitchDegrees) {
        m_Yaw = yawDegrees;
        m_Pitch = pitchDegrees;
        UpdateDirection();
    }
    
    void Camera::AddRotation(float yawDegrees, float pitchDegrees,bool constrainPitch) {
        m_Yaw += yawDegrees;
        m_Pitch += pitchDegrees;

        // 限制俯仰角在 -89 到 89 度之间
        if (constrainPitch) {
            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;
        }

        UpdateDirection();
    }

    const glm::vec3& Camera::GetPosition() const {
        return m_Position;
    }
    
    const std::tuple<glm::vec3, glm::vec3, glm::vec3> Camera::GetDirectionVectors() const {
        return std::make_tuple(m_Front, m_Up, m_Right);
    }
    
    void Camera::UpdateDirection() {
        // 根据欧拉角计算前向、右向和上向量
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Front = glm::normalize(front);
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

} // namespace graphics
