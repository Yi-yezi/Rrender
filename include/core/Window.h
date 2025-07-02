#pragma once
#include<GLFW/glfw3.h>
#include<iostream>
#include<string>

namespace core {

	class Window {
	public:
		Window(int height, int width, const std::string& title);
		~Window();

		// ��ֹ�����븳ֵ
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// �¼���ѯ + ˫����
		void PollEvents();
		void SwapBuffers();

		// ״̬��ѯ
		bool ShouldClose() const;
		int GetWidth() const;
		int GetHeight() const;

		// ��ȡԭʼ GLFWwindow*
		GLFWwindow* GetNativeHandle() const;

	private:
		GLFWwindow* m_window = nullptr;
		int m_width;
		int m_height;
		std::string m_title;
	};

}