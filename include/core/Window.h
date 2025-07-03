#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>

namespace core {

    /**
     * @brief 封装GLFW窗口的创建与管理
     * 
     * 该类负责初始化GLFW库、创建窗口、管理窗口生命周期，并提供窗口相关的常用操作接口。
     * 支持多窗口计数，确保GLFW只在第一个窗口创建时初始化，在最后一个窗口销毁时终结。
     */
    class Window {
    public:
        /**
         * @brief 构造函数，创建一个指定尺寸和标题的窗口
         * @param width  窗口宽度（像素）
         * @param height 窗口高度（像素）
         * @param title  窗口标题（显示在标题栏）
         * @throws std::runtime_error 创建窗口或初始化GLFW失败时抛出
         */
        Window(int width, int height, const std::string& title);

        /**
         * @brief 析构函数，销毁窗口并在最后一个窗口关闭时终结GLFW
         */
        ~Window();
    
        /**
         * @brief 事件轮询，处理窗口消息（如输入、关闭等）
         * 应在主循环每帧调用
         */
        void PollEvents();

        /**
         * @brief 交换前后缓冲区（OpenGL双缓冲），用于显示渲染结果
         */
        void SwapBuffers();

        /**
         * @brief 查询窗口是否应关闭（如用户点击关闭按钮）
         * @return true 需要关闭，false 继续运行
         */
        bool ShouldClose() const;

        /**
         * @brief 主动设置窗口关闭标志
         * @param flag true: 请求关闭窗口，false: 取消关闭
         */
        void SetShouldClose(bool flag);

        /**
         * @brief 获取窗口当前宽度（像素）
         */
        int GetWidth() const;

        /**
         * @brief 获取窗口当前高度（像素）
         */
        int GetHeight() const;

        /**
         * @brief 获取底层GLFW窗口指针
         * 可用于GLFW原生接口调用
         */
        GLFWwindow* GetNativeHandle() const;

    private:
        /**
         * @brief 初始化GLFW库（仅首次创建窗口时调用）
         * @throws std::runtime_error 初始化失败时抛出
         */
        static void InitGLFW();

        /**
         * @brief 终结GLFW库（最后一个窗口销毁时调用）
         */
        static void TerminateGLFW();

        static int s_WindowCount;    ///< 活跃窗口计数，确保GLFW只初始化/终结一次

        int m_width, m_height;       ///< 窗口尺寸（像素）
        std::string m_title;         ///< 窗口标题
        GLFWwindow* m_window = nullptr; ///< GLFW窗口指针
    };

} // namespace core
