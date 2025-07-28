#include "graphics/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace graphics {

    Camera::Camera(ProjectionType type) : m_ProjectionType(type) {
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

    void Camera::SetAspectRatio(float aspect) {
        m_Aspect = aspect;
        UpdateProjectionBounds(); // 更新正交投影边界
    }
    
    void Camera::SetPosition(const glm::vec3& pos) {
        m_Position = pos;
    }
    
    void Camera::SetRotation(float yawDegrees, float pitchDegrees) {
        m_Yaw = yawDegrees;
        m_Pitch = pitchDegrees;
        UpdateDirection();
    }
    
    void Camera::AddRotation(float yawDegrees, float pitchDegrees, bool constrainPitch) {
        m_Yaw += yawDegrees;
        m_Pitch += pitchDegrees;

        // 限制俯仰角在 -89 到 89 度之间
        if (constrainPitch) {
            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;
        }

        UpdateDirection();
    }

    void Camera::Move(CameraMovement direction, float deltaTime) {
        float velocity = m_MovementSpeed * deltaTime;
        switch (direction) {
            case CameraMovement::FORWARD:
                m_Position += m_Front * velocity;
                break;
            case CameraMovement::BACKWARD:
                m_Position -= m_Front * velocity;
                break;
            case CameraMovement::LEFT:
                m_Position -= m_Right * velocity;
                break;
            case CameraMovement::RIGHT:
                m_Position += m_Right * velocity;
                break;
            case CameraMovement::UP:
                m_Position += m_Up * velocity;
                break;
            case CameraMovement::DOWN:
                m_Position -= m_Up * velocity;
                break;
        }
    }

    void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
        xOffset *= m_MouseSensitivity;
        yOffset *= m_MouseSensitivity;
        
        AddRotation(xOffset, yOffset, constrainPitch);
    }

    void Camera::ProcessMouseScroll(float yOffset) {
        SetFOV(m_Fov - yOffset);
    }

    // Getters
    const glm::vec3& Camera::GetPosition() const {
        return m_Position;
    }

    const glm::vec3& Camera::GetFront() const {
        return m_Front;
    }

    const glm::vec3& Camera::GetUp() const {
        return m_Up;
    }

    const glm::vec3& Camera::GetRight() const {
        return m_Right;
    }
    
    const std::tuple<glm::vec3, glm::vec3, glm::vec3> Camera::GetDirectionVectors() const {
        return std::make_tuple(m_Front, m_Up, m_Right);
    }

    Camera::ProjectionType Camera::GetProjectionType() const {
        return m_ProjectionType;
    }

    float Camera::GetFOV() const {
        return m_Fov;
    }

    float Camera::GetAspectRatio() const {
        return m_Aspect;
    }

    float Camera::GetNearPlane() const {
        return m_Near;
    }

    float Camera::GetFarPlane() const {
        return m_Far;
    }

    float Camera::GetMovementSpeed() const {
        return m_MovementSpeed;
    }

    float Camera::GetMouseSensitivity() const {
        return m_MouseSensitivity;
    }

    float Camera::GetYaw() const {
        return m_Yaw;
    }

    float Camera::GetPitch() const {
        return m_Pitch;
    }

    // Setters
    void Camera::setProjectionType(ProjectionType type) {
        m_ProjectionType = type;
        UpdateProjectionBounds();
    }

    void Camera::SetFOV(float fov) {
        m_Fov = std::clamp(fov, 1.0f, 120.0f);
    }

    void Camera::SetNearFar(float nearPlane, float farPlane) {
        m_Near = std::max(nearPlane, 0.001f);
        m_Far = std::max(farPlane, m_Near + 0.1f);
        UpdateProjectionBounds();
    }

    void Camera::SetMovementSpeed(float speed) {
        m_MovementSpeed = std::max(speed, 0.1f);
    }

    void Camera::SetMouseSensitivity(float sensitivity) {
        m_MouseSensitivity = std::clamp(sensitivity, 0.01f, 1.0f);
    }

    void Camera::SetOrthographicBounds(float left, float right, float bottom, float top) {
        m_Left = left;
        m_RightBound = right;
        m_Bottom = bottom;
        m_Top = top;
    }

    void Camera::SetOrthographicSize(float size) {
        m_OrthographicSize = std::max(size, 0.1f);
        UpdateProjectionBounds();
    }

    // 私有辅助函数
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

    void Camera::UpdateProjectionBounds() {
        if (m_ProjectionType == ProjectionType::Orthographic) {
            float halfWidth = m_OrthographicSize * m_Aspect * 0.5f;
            float halfHeight = m_OrthographicSize * 0.5f;
            m_Left = -halfWidth;
            m_RightBound = halfWidth;
            m_Bottom = -halfHeight;
            m_Top = halfHeight;
        }
    }

} // namespace graphics
