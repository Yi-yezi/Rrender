#pragma once
#include "core/InputListener.h"
#include <memory>
#include "graphics/Camera.h"

namespace graphics {

class Camera;

class CameraController : public core::InputListener {
public:
    explicit CameraController(std::shared_ptr<Camera> camera);

    void OnInput() override;

    void SetCamera(std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> GetCamera() const;

    void SetMoveSpeed(float speed);
    void SetMouseSensitivity(float sensitivity);
    void SetPitchConstraint(bool enabled);

private:
    std::shared_ptr<Camera> m_Camera;

    float m_MoveSpeed = 3.0f;
    float m_MouseSensitivity = 0.1f;
    bool m_ConstrainPitch = true;
};

} // namespace graphics
