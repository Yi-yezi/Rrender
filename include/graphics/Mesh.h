#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace graphics {

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    class Mesh {
    public:
        // 新增 drawMode 参数，默认 triangles
        explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, GLenum drawMode = GL_TRIANGLES);
        explicit Mesh(const std::vector<Vertex>& vertices, GLenum drawMode = GL_TRIANGLES);
        ~Mesh();

        void Draw() const;

        // 禁拷贝，允许移动
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept;
        Mesh& operator=(Mesh&&) noexcept;

    private:
        unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
        size_t m_IndexCount = 0;
        bool m_UseEBO = true;
        GLsizei m_VertexCount = 0;
        GLenum m_DrawMode = GL_TRIANGLES; // 新增成员

        void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
        void SetupMesh(const std::vector<Vertex>& vertices);
    };

}
