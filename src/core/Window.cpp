#include "core/Window.h"
#include <algorithm>
#include <stdexcept>

namespace core {

    Window::Window(int width, int height, const std::string& title)
        : m_title(title), 
          m_uiWidthRatio(0.28f), m_minUIWidth(300.0f), m_maxUIWidth(500.0f) {

        InitGLFW(); // GLFW 初始化（仅当前实例）
        m_glfwInitialized = true;

        // 设置OpenGL上下文属性
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        m_window = glfwCreateWindow(width, height, m_title.c_str(), nullptr, nullptr);
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

    void Window::GetSize(int& width, int& height) const {
        glfwGetWindowSize(m_window, &width, &height);
    }

    void Window::GetFrameBufferSize(int& width, int& height) const {
        glfwGetFramebufferSize(m_window, &width, &height);
    }

    std::pair<int, int> Window::GetSize() const {
        int width, height;
        GetSize(width, height);
        return {width, height};
    }

    std::pair<int, int> Window::GetFrameBufferSize() const {
        int width, height;
        GetFrameBufferSize(width, height);
        return {width, height};
    }

    float Window::GetUIWidth() const {
        auto [width, height] = GetSize();
        float uiWidth = width * m_uiWidthRatio;
        
        // 小窗口时使用更大比例
        if (width < 1200.0f) {
            uiWidth = width * std::min(m_uiWidthRatio * 1.25f, 0.35f);
        }
        
        return std::clamp(uiWidth, m_minUIWidth, std::min(m_maxUIWidth, width * 0.5f));
    }

    float Window::GetRenderWidth() const {
        auto [width, height] = GetSize();
        return width - GetUIWidth();
    }

    float Window::GetRenderHeight() const {
        auto [width, height] = GetSize();
        return static_cast<float>(height);
    }

    void Window::SetUIWidthRatio(float ratio, float minWidth, float maxWidth) {
        m_uiWidthRatio = std::clamp(ratio, 0.1f, 0.5f);
        m_minUIWidth = minWidth;
        m_maxUIWidth = maxWidth;
    }

    bool Window::IsCompactMode() const {
        return GetUIWidth() < 350.0f;
    }

    float Window::GetAspectRatio() const {
        return GetRenderWidth() / GetRenderHeight();
    }

    GLFWwindow* Window::GetNativeHandle() const {
        return m_window;
    }

} // namespace core
