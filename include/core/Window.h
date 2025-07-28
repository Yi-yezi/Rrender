#pragma once
#include <string>
#include <utility>
#include <GLFW/glfw3.h>

namespace core {

class Window {
public:
    Window(int width=1920, int height=1080, const std::string& title="");
    ~Window();

    // 基本窗口操作
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;
    void SetShouldClose(bool flag);

    // 窗口大小相关
    void GetSize(int& width, int& height) const;
    void GetFrameBufferSize(int& width, int& height) const;
    std::pair<int, int> GetSize() const;
    std::pair<int, int> GetFrameBufferSize() const;
    
    // 响应式UI计算
    float GetUIWidth() const;
    float GetRenderWidth() const;
    float GetRenderHeight() const;
    float GetAspectRatio() const;
    bool IsCompactMode() const;
    void SetUIWidthRatio(float ratio, float minWidth = 300.0f, float maxWidth = 500.0f);

    // GLFW窗口句柄
    GLFWwindow* GetNativeHandle() const;

private:
    void InitGLFW();
    void TerminateGLFW();

    GLFWwindow* m_window;
    std::string m_title;
    bool m_glfwInitialized;

    // UI响应式参数
    float m_uiWidthRatio;
    float m_minUIWidth;
    float m_maxUIWidth;
};

} // namespace core
