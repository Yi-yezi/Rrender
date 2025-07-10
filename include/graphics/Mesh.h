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
        explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
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

        void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    };

}
