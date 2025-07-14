#pragma once
#include <memory>
#include "graphics/Shader.h"
#include "scene/Scene.h"
#include "graphics/Camera.h"

namespace pipeline {

class BasicPipeline {
public:
    explicit BasicPipeline(std::shared_ptr<graphics::Shader> shader);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera);

private:
    std::shared_ptr<graphics::Shader> m_Shader;
};

} // namespace pipeline
