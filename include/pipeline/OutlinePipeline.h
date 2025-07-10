#pragma once
#include <memory>
#include "graphics/Shader.h"
#include "scene/Scene.h"
#include "graphics/Camera.h"

namespace pipeline {

class OutlinePipeline {
public:
    OutlinePipeline(std::shared_ptr<graphics::Shader> baseShader,
                    std::shared_ptr<graphics::Shader> outlineShader);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera);

private:
    std::shared_ptr<graphics::Shader> m_baseShader;
    std::shared_ptr<graphics::Shader> m_outlineShader;
};

} // namespace pipeline
