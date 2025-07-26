#pragma once
#include <memory>
#include "scene/Scene.h"
#include "graphics/Camera.h"
#include "core/Window.h"

namespace pipeline {

class RenderPass {
public:
    virtual ~RenderPass() = default;

    /**
     * @brief 渲染场景主函数
     * @param scene 当前渲染场景
     * @param camera 当前相机
     */
    virtual void Execute(const std::shared_ptr<scene::Scene>& scene) = 0;
};

} // namespace pipeline