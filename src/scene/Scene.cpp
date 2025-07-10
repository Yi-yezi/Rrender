#include "scene/Scene.h"
#include <algorithm>

namespace scene {

void Scene::AddEntity(const std::shared_ptr<Entity>& entity) {
    if (entity && std::find(m_Entities.begin(), m_Entities.end(), entity) == m_Entities.end()) {
        m_Entities.push_back(entity);
    }
}

void Scene::RemoveEntity(const std::shared_ptr<Entity>& entity) {
    auto it = std::remove(m_Entities.begin(), m_Entities.end(), entity);
    if (it != m_Entities.end()) {
        m_Entities.erase(it, m_Entities.end());
    }
}

void Scene::AddLight(const std::shared_ptr<graphics::Light>& light) {
    if (light && std::find(m_Lights.begin(), m_Lights.end(), light) == m_Lights.end()) {
        m_Lights.push_back(light);
    }
}

void Scene::RemoveLight(const std::shared_ptr<graphics::Light>& light) {
    auto it = std::remove(m_Lights.begin(), m_Lights.end(), light);
    if (it != m_Lights.end()) {
        m_Lights.erase(it, m_Lights.end());
    }
}

const std::vector<std::shared_ptr<Entity>>& Scene::GetEntities() const {
    return m_Entities;
}

const std::vector<std::shared_ptr<graphics::Light>>& Scene::GetLights() const {
    return m_Lights;
}

} // namespace scene
