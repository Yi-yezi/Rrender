#pragma once

namespace core {

class InputListener {
public:
    virtual ~InputListener() = default;

    virtual void OnKey(int key, int scancode, int action, int mods) {}
    virtual void OnMouseButton(int button, int action, int mods) {}
    virtual void OnMouseMove(double xPos, double yPos, double deltaX, double deltaY) {}
    virtual void OnScroll(double xOffset, double yOffset) {}
};

}
