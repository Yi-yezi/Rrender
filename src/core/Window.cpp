#include "core/Window.h"

namespace core {

	int Window::s_WindowCount = 0;
	
	void Window::InitGLFW() {
	    if (!glfwInit()) {
	        throw std::runtime_error("Failed to initialize GLFW");
	    }
	}
	
	void Window::TerminateGLFW() {
	    glfwTerminate();
	}
	
	Window::Window(int width, int height, const std::string& title)
	    : m_width(width), m_height(height), m_title(title) {
	    if (s_WindowCount == 0) {
	        InitGLFW();
	    }
	    ++s_WindowCount;
	
	    // 设置OpenGL版本和profile
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	    // 创建窗口
	    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
	    if (!m_window) {
	        --s_WindowCount;
	        if (s_WindowCount == 0) {
	            TerminateGLFW();
	        }
	        throw std::runtime_error("Failed to create GLFW window");
	    }
	
	    glfwMakeContextCurrent(m_window);
	
	    // 开启垂直同步
	    glfwSwapInterval(1);
	}
	
	Window::~Window() {
	    if (m_window) {
	        glfwDestroyWindow(m_window);
	        --s_WindowCount;
	        if (s_WindowCount == 0) {
	            TerminateGLFW();
	        }
	    }
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
