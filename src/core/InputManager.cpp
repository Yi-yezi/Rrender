#include "core/InputManager.h"

namespace core{
    
    void InputManager::Init(GLFWwindow* window) {
        s_Window = window;
        // 注册GLFW回调函数
        glfwSetKeyCallback(s_Window, KeyCallback);
        glfwSetMouseButtonCallback(s_Window, MouseButtonCallback);
        glfwSetCursorPosCallback(s_Window, CursorPosCallback);

        // 初始化鼠标位置
        glfwGetCursorPos(window, &s_LastX, &s_LastY);
        s_MouseX = s_LastX;
        s_MouseY = s_LastY;
    }

    // TODO: 实现Update函数
    void InputManager::Update() {
        //
    }

    bool InputManager::IsKeyDown(int key) {
        return s_KeyState[key];
    }

    bool InputManager::IsKeyPressed(int key) {
        return s_KeyState[key] && !s_KeyStateLast[key];
    }

    bool InputManager::IsKeyReleased(int key) {
        return !s_KeyState[key] && s_KeyStateLast[key];
    }

    bool InputManager::IsMouseButtonDown(int button) {
        return s_MouseButtonState[button];
    }

    bool InputManager::IsMouseButtonPressed(int button) {
        return s_MouseButtonState[button] && !s_MouseButtonStateLast[button];
    }

    bool InputManager::IsMouseButtonReleased(int button) {
        return !s_MouseButtonState[button] && s_MouseButtonStateLast[button];
    }

    std::pair<double, double> InputManager::GetMousePosition() {
        return {s_MouseX, s_MouseY};
    }

    std::pair<double, double> InputManager::GetMouseDelta() {
        return {s_DeltaX, s_DeltaY};
    }

    void InputManager::RegisterListener(const std::shared_ptr<InputListener>& listener) {
        s_Listeners.push_back(listener);
    }

    void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            s_KeyState[key] = true;
        } 
        else if (action == GLFW_RELEASE) {
            s_KeyState[key] = false;
        }

        for (auto it = s_Listeners.begin(); it != s_Listeners.end();) {
            if (auto listener = it->lock()) {
                listener->OnKey(key, scancode, action, mods);
                ++it;
            } 
            else {
                it = s_Listeners.erase(it);
            }
        }
    }

    void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            s_MouseButtonState[button] = true;
        } else if (action == GLFW_RELEASE) {
            s_MouseButtonState[button] = false;
        }

        for (auto it = s_Listeners.begin(); it != s_Listeners.end();) {
            if (auto listener = it->lock()) {
                listener->OnMouseButton(button, action, mods);
                ++it;
            } 
            else {
                it = s_Listeners.erase(it);
            }
        }
    }

    void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        s_LastX = s_MouseX;
        s_LastY = s_MouseY;
        s_MouseX = xpos;
        s_MouseY = ypos;
        s_DeltaX = s_MouseX - s_LastX;
        s_DeltaY = s_MouseY - s_LastY;
        for (auto it = s_Listeners.begin(); it != s_Listeners.end();) {
            if (auto listener = it->lock()) {
                listener->OnMouseMove(xpos, ypos, s_DeltaX, s_DeltaY);
                ++it;
            } else {
                it = s_Listeners.erase(it);
            }
        }
    }

    void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        for (auto it = s_Listeners.begin(); it != s_Listeners.end();) {
            if (auto listener = it->lock()) {
                listener->OnScroll(xoffset, yoffset);
                ++it;
            } else {
                it = s_Listeners.erase(it);
            }
        }
    }

}