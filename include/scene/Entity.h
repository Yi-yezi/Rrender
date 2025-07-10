#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/Model.h"

namespace scene {

    /**
     * @brief 场景中一个实体对象，包含模型和变换（位置、旋转、缩放）
     */
    class Entity {
    public:
        explicit Entity(std::shared_ptr<graphics::Model> model);

        // 设置 / 获取模型
        void SetModel(std::shared_ptr<graphics::Model> model);
        std::shared_ptr<graphics::Model> GetModel() const;

        // 设置 / 获取位置
        void SetPosition(const glm::vec3& position);
        const glm::vec3& GetPosition() const;

        // 设置 / 获取旋转（欧拉角，单位为度）
        void SetRotation(const glm::vec3& rotation);
        const glm::vec3& GetRotation() const;

        // 设置 / 获取缩放
        void SetScale(const glm::vec3& scale);
        const glm::vec3& GetScale() const;

        // 获取模型矩阵（最终变换矩阵）
        glm::mat4 GetModelMatrix() const;

        // 绘制模型
        void Draw() const;

    private:
        std::shared_ptr<graphics::Model> m_Model;

        glm::vec3 m_Position{0.0f};
        glm::vec3 m_Rotation{0.0f};  // pitch, yaw, roll
        glm::vec3 m_Scale{1.0f};
    };

} // namespace scene
