#include "core/Window.h"

namespace core {

    Window::Window(int width, int height, const std::string& title)
        : m_width(width), m_height(height), m_title(title) {

        InitGLFW(); // GLFW 初始化（仅当前实例）
        m_glfwInitialized = true;

        // 设置OpenGL上下文属性
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable V-Sync
    }

    Window::~Window() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        if (m_glfwInitialized) {
            TerminateGLFW(); // 当前窗口负责释放GLFW
            m_glfwInitialized = false;
        }
    }

    void Window::InitGLFW() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
    }

    void Window::TerminateGLFW() {
        glfwTerminate();
    }

    void Window::PollEvents() {
        glfwPollEvents();
    }

    void Window::SwapBuffers() {
        glfwSwapBuffers(m_window);
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

    void Window::SetShouldClose(bool flag) {
        glfwSetWindowShouldClose(m_window, flag);
    }

    int Window::GetWidth() const {
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        return width;
    }

    int Window::GetHeight() const {
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        return height;
    }

    GLFWwindow* Window::GetNativeHandle() const {
        return m_window;
    }

} // namespace core
