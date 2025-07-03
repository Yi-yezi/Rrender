#pragma once
#include<GLFW/glfw3.h>
#include<iostream>
#include<string>

namespace core {
    /**
     * @brief 封装GLFW窗口的创建与管理
     */
    class Window {
    public:
        /**
         * @brief 创建窗口
         * @param height 高度
         * @param width 宽度
         * @param title 标题
         */
        Window(int height, int width, const std::string& title);

        ~Window();

        // 禁止拷贝与赋值
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void PollEvents();      ///< 事件轮询
        void SwapBuffers();     ///< 交换缓冲区
        bool ShouldClose() const; ///< 窗口是否应关闭
        int GetWidth() const;     ///< 获取宽度
        int GetHeight() const;    ///< 获取高度
        GLFWwindow* GetNativeHandle() const; ///< 获取GLFW窗口指针

    private:
        GLFWwindow* m_window = nullptr; // GLFW窗口指针
        int m_width;                    // 宽度
        int m_height;                   // 高度
        std::string m_title;            // 标题
    };

}