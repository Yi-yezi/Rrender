#pragma once

#include <memory>
#include <string>
#include "graphics/Camera.h"
#include "scene/Scene.h"
#include "core/Window.h"
#include "pipeline/BRDFPipeline.h"

namespace ui {

class UIManager {
public:
    static void Init(std::weak_ptr<core::Window> windowPtr, const char* glslVersion = "#version 330");
    static void Shutdown();

    static void BeginFrame();
    static void EndFrame();

    static void RenderUI(const std::shared_ptr<graphics::Camera> &camera,
                         const std::shared_ptr<scene::Scene> &scene,
                         const std::shared_ptr<core::Window>& windowPtr,
                         const std::shared_ptr<pipeline::BRDFPipeline> &brdfPipeline=nullptr);

private:
    static bool s_Initialized;
};

} // namespace ui
