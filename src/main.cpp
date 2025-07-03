#include <iostream>
#include <memory>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "core/Window.h"
#include "core/InputManager.h"
#include "graphics/Camera.h"
#include "graphics/CameraController.h"
#include "utils/Time.h"

using namespace core;
using namespace graphics;

// 顶点着色器源码
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uView;
uniform mat4 uProj;
void main() {
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
)";

// 片元着色器源码
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

// 编译着色器工具函数
GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Compile Error: " << infoLog << std::endl;
    }
    return shader;
}

int main() {
    try {
        auto windowPtr = std::make_shared<Window>(800, 600, "Cube Test");
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD\n";
            return -1;
        }

        glEnable(GL_DEPTH_TEST); // 启用深度测试

        InputManager inputManager(windowPtr);

        auto cameraPtr = std::make_shared<Camera>(Camera::ProjectionType::Perspective);
        auto controllerPtr = std::make_shared<CameraController>(cameraPtr);
        inputManager.RegisterListener(controllerPtr);

        // 立方体顶点数据（36个顶点，12个三角形）
        float cubeVertices[] = {
            // 前面
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            // 后面
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            // 左面
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // 右面
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             // 上面
             -0.5f,  0.5f,  0.5f,
              0.5f,  0.5f,  0.5f,
              0.5f,  0.5f, -0.5f,
              0.5f,  0.5f, -0.5f,
             -0.5f,  0.5f, -0.5f,
             -0.5f,  0.5f,  0.5f,
             // 下面
             -0.5f, -0.5f,  0.5f,
             -0.5f, -0.5f, -0.5f,
              0.5f, -0.5f, -0.5f,
              0.5f, -0.5f, -0.5f,
              0.5f, -0.5f,  0.5f,
             -0.5f, -0.5f,  0.5f
        };

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 编译着色器
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // 主循环
        while (!windowPtr->ShouldClose()) {
            windowPtr->PollEvents();
            utils::Time::Update(glfwGetTime());
            inputManager.Update();

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderProgram);

            // 设置相机矩阵
            glm::mat4 view = cameraPtr->GetViewMatrix();
            glm::mat4 proj = cameraPtr->GetProjectionMatrix();
            GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");
            GLint projLoc = glGetUniformLocation(shaderProgram, "uProj");
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            windowPtr->SwapBuffers();
        }

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
