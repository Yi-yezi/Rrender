#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/Model.h"
#include "graphics/Shader.h"

namespace scene {
    
    /**
     * @brief 场景中的一个实体，包含模型和变换信息
     */
    class Entity {
    public:
        Entity() = default;
        explicit Entity(std::shared_ptr<graphics::Model> model);
        
        // 模型管理
        void SetModel(std::shared_ptr<graphics::Model> model);
        std::shared_ptr<graphics::Model> GetModel() const;
        
        // 变换操作
        void SetPosition(const glm::vec3& position);
        const glm::vec3& GetPosition() const;
        
        void SetRotation(const glm::vec3& rotation);
        const glm::vec3& GetRotation() const;
        
        void SetScale(const glm::vec3& scale);
        const glm::vec3& GetScale() const;
        
        // 获取组合变换矩阵
        glm::mat4 GetModelMatrix() const;
        
        /**
         * @brief 绘制实体（推荐方式：使用 Material 系统）
         * @param shader 用于渲染的着色器程序
         */
        void Draw(std::shared_ptr<graphics::Shader> shader) const;
        
        
    private:
        std::shared_ptr<graphics::Model> m_Model = nullptr;
        
        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Rotation = glm::vec3(0.0f);  // 欧拉角（度）
        glm::vec3 m_Scale = glm::vec3(1.0f);
    };

} // namespace scene
