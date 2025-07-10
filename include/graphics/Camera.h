#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>

namespace graphics {

/**
 * @brief 通用相机类，支持透视与正交投影
 */
class Camera {
public:
    /**
     * @brief 投影类型
     */
    enum class ProjectionType {
        Perspective,   ///< 透视投影
        Orthographic   ///< 正交投影
    };

    Camera(ProjectionType type);

    /**
     * @brief 设置投影类型
     * @param type 投影类型
     */
    void setProjectionType(ProjectionType type) {
        m_ProjectionType = type;
    }

    /**
     * @brief 获取当前投影类型
     * @return 当前投影类型
     */
    ProjectionType GetProjectionType() const {
        return m_ProjectionType;
    }

    /**
     * @brief 获取视图矩阵
     */
    glm::mat4 GetViewMatrix() const;

    /**
     * @brief 获取投影矩阵
     */
    glm::mat4 GetProjectionMatrix() const;

    /**
     * @brief 设置相机位置
     */
    void SetPosition(const glm::vec3& pos);

    /**
     * @brief 设置相机欧拉角（旋转）
     * @param yawDegrees 偏航角
     * @param pitchDegrees 俯仰角
     */
    void SetRotation(float yawDegrees, float pitchDegrees);

    /**
     * @brief 增加相机旋转
     * @param yawDegrees 偏航角增量
     * @param pitchDegrees 俯仰角增量
     */
    void AddRotation(float yawDegrees, float pitchDegrees, bool constrainPitch);

    /**
     * @brief 获取相机位置
     */
    const glm::vec3& GetPosition() const;

    /**
     * @brief 获取相机前向量
     */
    const glm::vec3& GetFront() const;

    /**
     * @brief 获取相机上向量
     */
    const glm::vec3& GetUp() const;

    /**
     * @brief 获取相机右向量
     */
    const glm::vec3& GetRight() const;

    /**
     * @brief 获取相机方向向量（前、上、右）
     * @return std::tuple 包含前向量、上向量和右向量
     */
    const std::tuple<glm::vec3, glm::vec3, glm::vec3> GetDirectionVectors() const;

    /**
     * @brief 更新相机方向向量，调用SetRotation后必须调用
     */
    void UpdateDirection();

private:
    ProjectionType m_ProjectionType = ProjectionType::Perspective; ///< 当前投影类型

    glm::vec3 m_Position = glm::vec3(0.f);   ///< 相机位置
    glm::vec3 m_Front = glm::vec3(0.f, 0.f, -1.f); ///< 前向量
    glm::vec3 m_Up = glm::vec3(0.f, 1.f, 0.f);     ///< 上向量
    glm::vec3 m_Right = glm::vec3(1.f, 0.f, 0.f);  ///< 右向量
    glm::vec3 m_WorldUp = glm::vec3(0.f, 1.f, 0.f);///< 世界上向量

    // 欧拉角
    float m_Yaw = -90.f;   ///< 偏航角
    float m_Pitch = 0.f;   ///< 俯仰角

    // 透视投影参数
    float m_Fov = 45.f;    ///< 视野角度
    float m_Aspect = 1.f;  ///< 宽高比
    float m_Near = 0.1f;   ///< 近平面
    float m_Far = 100.f;   ///< 远平面

    // 正交投影参数
    float m_Left = -1.f;       ///< 左边界
    float m_RightBound = 1.f;  ///< 右边界
    float m_Bottom = -1.f;     ///< 下边界
    float m_Top = 1.f;         ///< 上边界

    mutable glm::mat4 m_ProjectionMatrix = glm::mat4(1.f); ///< 投影矩阵缓存，允许在const函数中更新
};

}
