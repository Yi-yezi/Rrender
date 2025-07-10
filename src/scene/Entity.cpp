#include "scene/Entity.h"

namespace scene {

    Entity::Entity(std::shared_ptr<graphics::Model> model)
        : m_Model(std::move(model)) {}

    void Entity::SetModel(std::shared_ptr<graphics::Model> model) {
        m_Model = std::move(model);
    }

    std::shared_ptr<graphics::Model> Entity::GetModel() const {
        return m_Model;
    }

    void Entity::SetPosition(const glm::vec3& position) {
        m_Position = position;
    }

    const glm::vec3& Entity::GetPosition() const {
        return m_Position;
    }

    void Entity::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
    }

    const glm::vec3& Entity::GetRotation() const {
        return m_Rotation;
    }

    void Entity::SetScale(const glm::vec3& scale) {
        m_Scale = scale;
    }

    const glm::vec3& Entity::GetScale() const {
        return m_Scale;
    }

    glm::mat4 Entity::GetModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_Position);
        model = glm::rotate(model, glm::radians(m_Rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(m_Rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, m_Scale);
        return model;
    }

    void Entity::Draw() const {
        if (m_Model) {
            m_Model->Draw();
        }
    }

}
