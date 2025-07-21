#include "graphics/Mesh.h"
#include <iostream>

namespace graphics {

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, GLenum drawMode)
        : m_DrawMode(drawMode) {

        SetupMesh(vertices, indices);
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, GLenum drawMode)
        : m_DrawMode(drawMode) {
        SetupMesh(vertices);
    }


    Mesh::~Mesh() {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    // 修改后的移动构造函数
    Mesh::Mesh(Mesh&& other) noexcept 
        : m_VAO(other.m_VAO),
          m_VBO(other.m_VBO),
          m_EBO(other.m_EBO),
          m_IndexCount(other.m_IndexCount),
          m_VertexCount(other.m_VertexCount),
          m_UseEBO(other.m_UseEBO),
          m_DrawMode(other.m_DrawMode) // 修正：移动drawMode
    {
        // 置空源对象
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_IndexCount = 0;
        other.m_VertexCount = 0;
        other.m_UseEBO = false;
        other.m_DrawMode = GL_TRIANGLES; // 可选，重置为默认
    }

    // 修改后的移动赋值运算符
    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            // 释放当前对象资源
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);

            // 转移资源
            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_EBO = other.m_EBO;
            m_IndexCount = other.m_IndexCount;
            m_VertexCount = other.m_VertexCount;
            m_UseEBO = other.m_UseEBO;
            m_DrawMode = other.m_DrawMode; // 修正：移动drawMode

            // 置空源对象
            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_EBO = 0;
            other.m_IndexCount = 0;
            other.m_VertexCount = 0;
            other.m_UseEBO = false;
            other.m_DrawMode = GL_TRIANGLES; // 可选，重置为默认
        }
        return *this;
    }

    void Mesh::SetupMesh(const std::vector<Vertex>& vertices) {
        m_UseEBO = false;
        m_VertexCount = static_cast<GLsizei>(vertices.size());

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }

    void Mesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        m_UseEBO = true;
        m_IndexCount = indices.size();

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        glBindVertexArray(m_VAO);
        if (m_UseEBO)
            glDrawElements(m_DrawMode, static_cast<GLsizei>(m_IndexCount), GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(m_DrawMode, 0, m_VertexCount);
        glBindVertexArray(0);
    }

}
