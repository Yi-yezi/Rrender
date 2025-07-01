#include "core/Window.h"
#include <iostream>
#include <cassert>

namespace core {

	Window::Window(int height, int width, const std::string& title)
		: m_height(height), m_width(width), m_title(title) {
		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialize GLFW");
		}

		// OpenGL 3.3 Core Profile
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
		if (!m_window) {
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1); // 启用垂直同步 V-sync
	}

	Window::~Window() {
		if (m_window) {
			glfwDestroyWindow(m_window);
		}
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

	int Window::GetWidth() const {
		return m_width;
	}

	int Window::GetHeight() const {
		return m_height;
	}

	GLFWwindow* Window::GetNativeHandle() const {
		return m_window;
	}

}