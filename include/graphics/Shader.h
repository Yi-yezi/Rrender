#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace graphics {

    /**
     * @brief OpenGL Shader 封装类，符合 RAII 原则，负责加载、编译、绑定、设置 uniform
     */
    class Shader {
    public:
        /**
         * @brief 构造函数，立即加载 shader（推荐）
         * @param vertexPath 顶点着色器路径
         * @param fragmentPath 片段着色器路径
         */
        Shader(const std::string& vertexPath, const std::string& fragmentPath);

        /**
         * @brief 禁止拷贝构造和赋值
         */
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        /**
         * @brief 允许移动构造和移动赋值
         */
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        /**
         * @brief 析构函数，自动释放 OpenGL 资源
         */
        ~Shader();

        /**
         * @brief 绑定 Shader 程序
         */
        void Bind() const;

        /**
         * @brief 解绑 Shader 程序
         */
        void Unbind() const;

        /// 设置 uniform（支持多种类型）
        void SetUniform(const std::string& name, int value);
        void SetUniform(const std::string& name, float value);
        void SetUniform(const std::string& name, const glm::vec2& value);
        void SetUniform(const std::string& name, const glm::vec3& value);
        void SetUniform(const std::string& name, const glm::vec4& value);
        void SetUniform(const std::string& name, const glm::mat4& value);

        /**
         * @brief 获取 shader 程序 ID
         */
        unsigned int GetID() const { return m_ID; }

    private:
        unsigned int m_ID = 0;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;

        std::string ReadFile(const std::string& path) const;
        unsigned int CompileShader(unsigned int type, const std::string& source) const;
        void CheckCompileErrors(unsigned int shader, const std::string& type) const;
        int GetUniformLocation(const std::string& name) const;
    };

} // namespace graphics
