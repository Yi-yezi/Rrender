#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include "InputListener.h"
#include "Window.h"

namespace core {

class InputManager {
public:
    // 静态初始化函数，绑定回调，传入 Window 引用
    static void Init(std::shared_ptr<Window> window);

    static void Update();

    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);

    static bool IsMouseButtonDown(int button);
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonReleased(int button);

    static float GetAspectRatio();

    static std::pair<double, double> GetMousePosition();
    static std::pair<double, double> GetMouseDelta();

    static void RegisterListener(const std::shared_ptr<InputListener>& listener);

private:
    // GLFW回调
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void viewportSizeCallback(GLFWwindow* window, int width, int height);

    // 静态变量存储状态
    inline static std::weak_ptr<Window> s_Window;
    inline static GLFWwindow* s_GLFWWindow = nullptr;

    inline static std::unordered_map<int, bool> s_KeyState;
    inline static std::unordered_map<int, bool> s_KeyStateLast;

    inline static std::unordered_map<int, bool> s_MouseButtonState;
    inline static std::unordered_map<int, bool> s_MouseButtonStateLast;

    inline static double s_MouseX = 0.0, s_MouseY = 0.0;
    inline static double s_LastX = 0.0, s_LastY = 0.0;
    inline static double s_DeltaX = 0.0, s_DeltaY = 0.0;
    inline static double s_ScrollX = 0.0, s_ScrollY = 0.0;
    inline static bool s_FirstMouseMove = true;
    inline static float aspectRatio = 1.0f; // 默认宽高比

    inline static std::vector<std::weak_ptr<InputListener>> s_Listeners;
};

} // namespace core
