#pragma once
#include <memory>
#include "scene/Scene.h"
#include "graphics/Camera.h"

namespace pipeline {

class RenderPipeline {
public:
    virtual ~RenderPipeline() = default;

    /**
     * @brief 渲染场景主函数
     * @param scene 当前渲染场景
     * @param camera 当前相机
     */
    virtual void Render(const std::shared_ptr<scene::Scene>& scene,
                        const std::shared_ptr<graphics::Camera>& camera) = 0;
};

} // namespace pipeline
