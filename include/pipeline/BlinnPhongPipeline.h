#pragma once
#include "pipeline/RenderPipeline.h"
#include "graphics/Shader.h"
#include <memory>

namespace pipeline {

class BlinnPhongPipeline : public RenderPipeline {
public:
    explicit BlinnPhongPipeline(std::shared_ptr<graphics::Shader> shader);

    void Render(const std::shared_ptr<scene::Scene>& scene,
                const std::shared_ptr<graphics::Camera>& camera) override;

private:
    std::shared_ptr<graphics::Shader> m_Shader;
};

} // namespace pipeline
