#pragma once

#include <vector>
#include <memory>
#include "scene/Entity.h"
#include "graphics/Light.h"

namespace scene {

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    // 禁止拷贝，允许移动
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    // 添加实体
    void AddEntity(const std::shared_ptr<Entity>& entity);
    void RemoveEntity(const std::shared_ptr<Entity>& entity);

    // 添加光源
    void AddLight(const std::shared_ptr<graphics::Light>& light);
    void RemoveLight(const std::shared_ptr<graphics::Light>& light);

    // 获取所有实体和光源
    const std::vector<std::shared_ptr<Entity>>& GetEntities() const;
    const std::vector<std::shared_ptr<graphics::Light>>& GetLights() const;

private:
    std::vector<std::shared_ptr<Entity>> m_Entities;
    std::vector<std::shared_ptr<graphics::Light>> m_Lights;
};

} // namespace scene
