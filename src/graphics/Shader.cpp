#include "graphics/Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace graphics {

    // 构造函数：加载并编译着色器
    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexCode = ReadFile(vertexPath);
        std::string fragmentCode = ReadFile(fragmentPath);

        unsigned int vertex = CompileShader(GL_VERTEX_SHADER, vertexCode);
        unsigned int fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);

        // 创建程序对象并链接
        m_ID = glCreateProgram();
        glAttachShader(m_ID, vertex);
        glAttachShader(m_ID, fragment);
        glLinkProgram(m_ID);
        CheckCompileErrors(m_ID, "PROGRAM");

        // 删除已链接的着色器对象
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // 移动构造函数
    Shader::Shader(Shader&& other) noexcept {
        m_ID = other.m_ID;
        m_UniformLocationCache = std::move(other.m_UniformLocationCache);
        other.m_ID = 0;
    }

    // 移动赋值运算符
    Shader& Shader::operator=(Shader&& other) noexcept {
        if (this != &other) {
            glDeleteProgram(m_ID);
            m_ID = other.m_ID;
            m_UniformLocationCache = std::move(other.m_UniformLocationCache);
            other.m_ID = 0;
        }
        return *this;
    }

    // 析构函数：释放OpenGL程序对象
    Shader::~Shader() {
        if (m_ID != 0) {
            glDeleteProgram(m_ID);
        }
    }

    // 绑定着色器程序
    void Shader::Bind() const {
        glUseProgram(m_ID);
    }

    // 解绑着色器程序
    void Shader::Unbind() const {
        glUseProgram(0);
    }

    // 设置int类型uniform
    void Shader::SetUniform(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }

    // 设置float类型uniform
    void Shader::SetUniform(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }

    // 设置vec2类型uniform
    void Shader::SetUniform(const std::string& name, const glm::vec2& value) {
        glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    }

    // 设置vec3类型uniform
    void Shader::SetUniform(const std::string& name, const glm::vec3& value) {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    // 设置vec4类型uniform
    void Shader::SetUniform(const std::string& name, const glm::vec4& value) {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    // 设置mat4类型uniform
    void Shader::SetUniform(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    // 读取文件内容为字符串
    std::string Shader::ReadFile(const std::string& path) const {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // 编译单个着色器
    unsigned int Shader::CompileShader(unsigned int type, const std::string& source) const {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        CheckCompileErrors(id, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        return id;
    }

    // 检查着色器编译或程序链接错误
    void Shader::CheckCompileErrors(unsigned int shader, const std::string& type) const {
        int success;
        char infoLog[1024];

        if (type == "PROGRAM") {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "[SHADER::LINK::ERROR] " << infoLog << std::endl;
                throw std::runtime_error("Shader program linking failed.");
            }
        } else {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                std::cerr << "[SHADER::COMPILE::" << type << "] " << infoLog << std::endl;
                throw std::runtime_error("Shader compilation failed.");
            }
        }
    }

    // 获取uniform变量位置（带缓存）
    int Shader::GetUniformLocation(const std::string& name) const {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
            return m_UniformLocationCache[name];
        }
        int location = glGetUniformLocation(m_ID, name.c_str());
        if (location == -1) {
            std::cerr << "[WARNING] Uniform '" << name << "' not found or not used." << std::endl;
        }
        m_UniformLocationCache[name] = location;
        return location;
    }

} // namespace graphics
