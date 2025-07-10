#include "core/InputManager.h"

namespace core {

void InputManager::Init(std::shared_ptr<Window> window) {
    s_Window = window;
    if (auto windowPtr = s_Window.lock()) {
        s_GLFWWindow = windowPtr->GetNativeHandle();

        glfwSetKeyCallback(s_GLFWWindow, KeyCallback);
        glfwSetMouseButtonCallback(s_GLFWWindow, MouseButtonCallback);
        glfwSetCursorPosCallback(s_GLFWWindow, CursorPosCallback);
        glfwSetScrollCallback(s_GLFWWindow, ScrollCallback);
        glfwSetInputMode(s_GLFWWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //glfwSetFramebufferSizeCallback(s_GLFWWindow, viewportSizeCallback);
    }
}

void InputManager::Update() {
    int mode = glfwGetInputMode(s_GLFWWindow, GLFW_CURSOR);
    if (mode == GLFW_CURSOR_DISABLED) {
        for (auto it = s_Listeners.begin(); it != s_Listeners.end();) {
            if (auto listener = it->lock()) {
                listener->OnInput();
                ++it;
            } else {
                it = s_Listeners.erase(it);
            }
        }
    }
    

    s_KeyStateLast = s_KeyState;
    s_MouseButtonStateLast = s_MouseButtonState;

    s_DeltaX = 0.0;
    s_DeltaY = 0.0;
    s_ScrollX = 0.0;
    s_ScrollY = 0.0;
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

void InputManager::viewportSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

float InputManager::GetAspectRatio() {
    if (auto windowPtr = s_Window.lock()) {
        int width, height;
        windowPtr->GetFrameBufferSize(width, height);
        if (height == 0) return 1.0f; // 防止除以零
        return static_cast<float>(width) / static_cast<float>(height);
    }
    return aspectRatio; // 如果窗口不可用，返回默认值
}

void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto windowPtr = s_Window.lock();
    if (!windowPtr) return;

    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        windowPtr->SetShouldClose(true);
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
        int mode = glfwGetInputMode(window, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    s_KeyState[key] = (action != GLFW_RELEASE);
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    s_MouseButtonState[button] = (action != GLFW_RELEASE);
}

void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_FirstMouseMove) {
        s_LastX = xpos;
        s_LastY = ypos;
        s_FirstMouseMove = false;
    }
    s_DeltaX += xpos - s_LastX;
    s_DeltaY += s_LastY - ypos;
    s_LastX = xpos;
    s_LastY = ypos;
    s_MouseX = xpos;
    s_MouseY = ypos;
}

void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    s_ScrollX = xoffset;
    s_ScrollY = yoffset;
}

} // namespace core
