#pragma once
#include<GLFW/glfw3.h>
#include<iostream>
#include<string>

namespace core {

	class Window {
	public:
		Window(int height, int width, const std::string& title);
		~Window();

		// 禁止拷贝与赋值
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// 事件轮询 + 双缓冲
		void PollEvents();
		void SwapBuffers();

		// 状态查询
		bool ShouldClose() const;
		int GetWidth() const;
		int GetHeight() const;

		// 获取原始 GLFWwindow*
		GLFWwindow* GetNativeHandle() const;

	private:
		GLFWwindow* m_window = nullptr;
		int m_width;
		int m_height;
		std::string m_title;
	};

}