#pragma once
#include <glm/glm.hpp>
#include <tuple>

namespace graphics {

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    Camera(ProjectionType type = ProjectionType::Perspective);

    // 矩阵获取
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // 变换操作
    void SetPosition(const glm::vec3& pos);
    void SetRotation(float yawDegrees, float pitchDegrees);
    void AddRotation(float yawDegrees, float pitchDegrees, bool constrainPitch = true);
    void Move(CameraMovement direction, float deltaTime);

    // 输入处理
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yOffset);

    // Getters
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetFront() const;
    const glm::vec3& GetUp() const;
    const glm::vec3& GetRight() const;
    const std::tuple<glm::vec3, glm::vec3, glm::vec3> GetDirectionVectors() const;
    
    ProjectionType GetProjectionType() const;
    float GetFOV() const;
    float GetAspectRatio() const;
    float GetNearPlane() const;
    float GetFarPlane() const;
    float GetMovementSpeed() const;
    float GetMouseSensitivity() const;
    float GetYaw() const;
    float GetPitch() const;

    // Setters
    void setProjectionType(ProjectionType type);
    void SetFOV(float fov);
    void SetAspectRatio(float aspect);
    void SetNearFar(float nearPlane, float farPlane);
    void SetMovementSpeed(float speed);
    void SetMouseSensitivity(float sensitivity);
    void SetOrthographicBounds(float left, float right, float bottom, float top);
    void SetOrthographicSize(float size);

private:
    void UpdateDirection();
    void UpdateProjectionBounds();

    // 位置和方向
    glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // 欧拉角
    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;

    // 投影参数
    ProjectionType m_ProjectionType = ProjectionType::Perspective;
    float m_Fov = 45.0f;
    float m_Aspect = 16.0f / 9.0f;
    float m_Near = 0.1f;
    float m_Far = 100.0f;

    // 正交投影参数
    float m_Left = -1.0f;
    float m_RightBound = 1.0f;
    float m_Bottom = -1.0f;
    float m_Top = 1.0f;
    float m_OrthographicSize = 10.0f;

    // 控制参数
    float m_MovementSpeed = 2.5f;
    float m_MouseSensitivity = 0.1f;
};

} // namespace graphics
