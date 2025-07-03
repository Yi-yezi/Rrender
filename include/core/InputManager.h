#pragma once
#include<GLFW/glfw3.h>
#include<memory>
#include<unordered_map>
#include<vector>
#include "InputListener.h"
#include "Window.h"

namespace core {
    /**
     * @brief 输入管理器，处理键盘和鼠标输入
     */
    class InputManager {
        public:
            /**
             * @brief 初始化输入管理器
             * @param window 窗口智能指针
             */
            explicit InputManager(std::shared_ptr<Window> window);

            /**
             * @brief 更新输入状态，通常在每帧调用
             * 需要在渲染循环中调用以更新按键状态
             */
            static void Update(); // TODO：暂未实现

            /**
             * @brief 检查某键是否处于按下状态（持续）
             * @param key GLFW键码
             * @return true 按下，false 未按下
             */
            static bool IsKeyDown(int key);

            /**
             * @brief 检查某键是否在本帧被按下（边缘触发）
             * @param key GLFW键码
             * @return true 本帧按下，false 否
             */
            static bool IsKeyPressed(int key);

            /**
             * @brief 检查某键是否在本帧被释放
             * @param key GLFW键码
             * @return true 本帧释放，false 否
             */
            static bool IsKeyReleased(int key);

            /**
             * @brief 检查鼠标某键是否处于按下状态
             * @param button 鼠标按钮（GLFW_MOUSE_BUTTON_*）
             * @return true 按下，false 未按下
             */
            static bool IsMouseButtonDown(int button);

            /**
             * @brief 检查鼠标某键是否在本帧被按下
             * @param button 鼠标按钮
             * @return true 本帧按下，false 否
             */
            static bool IsMouseButtonPressed(int button);

            /**
             * @brief 检查鼠标某键是否在本帧被释放
             * @param button 鼠标按钮
             * @return true 本帧释放，false 否
             */
            static bool IsMouseButtonReleased(int button);

            /**
             * @brief 获取当前鼠标位置
             * @return (x, y) 坐标对
             */
            static std::pair<double, double> GetMousePosition();

            /**
             * @brief 获取鼠标本帧移动的距离
             * @return (dx, dy) 坐标差
             */
            static std::pair<double, double> GetMouseDelta();

            /**
             * @brief 注册输入监听器
             * @param listener 监听器智能指针
             */
            static void RegisterListener(const std::shared_ptr<InputListener>& listener);

        private:
            /**
             * @brief GLFW键盘事件回调
             */
            static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

            /**
             * @brief GLFW鼠标按键事件回调
             */
            static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

            /**
             * @brief GLFW鼠标移动事件回调
             */
            static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

            /**
             * @brief GLFW鼠标滚轮事件回调
             */
            static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


            inline static std::shared_ptr<Window> s_Window; // 窗口指针

            inline static std::unordered_map<int, bool> s_KeyState;         // 当前帧键盘状态
            inline static std::unordered_map<int, bool> s_KeyStateLast;     // 上一帧键盘状态

            inline static std::unordered_map<int, bool> s_MouseButtonState;     // 当前帧鼠标按键状态
            inline static std::unordered_map<int, bool> s_MouseButtonStateLast; // 上一帧鼠标按键状态

            inline static double s_MouseX = 0.0, s_MouseY = 0.0;     // 当前鼠标坐标
            inline static double s_LastX = 0.0, s_LastY = 0.0;       // 上一帧鼠标坐标
            inline static double s_DeltaX = 0.0, s_DeltaY = 0.0;     // 鼠标本帧移动距离
            inline static double s_ScrollX = 0.0, s_ScrollY = 0.0;   // 鼠标滚轮偏移
			inline static bool firstMouseMove = true; // 是否第一次鼠标移动

            inline static std::vector<std::weak_ptr<InputListener>> s_Listeners; // 输入监听器

    };

}