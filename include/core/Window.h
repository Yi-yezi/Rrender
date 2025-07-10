#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

namespace core {

    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        void PollEvents();
        void SwapBuffers();

        bool ShouldClose() const;
        void SetShouldClose(bool flag);

        int GetWidth() const;
        int GetHeight() const;

        GLFWwindow* GetNativeHandle() const;

    private:
        void InitGLFW();
        void TerminateGLFW();

        int m_width, m_height;
        std::string m_title;
        GLFWwindow* m_window = nullptr;
        bool m_glfwInitialized = false;  ///< 当前实例是否初始化GLFW
    };

} // namespace core
