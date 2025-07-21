#include "graphics/Mesh.h"

namespace utils {
std::vector<graphics::Vertex> ParseVertexData(const float* rawData, size_t count) {
    std::vector<graphics::Vertex> vertices;
    for (size_t i = 0; i < count; ++i) {
        graphics::Vertex vertex;
        vertex.Position = glm::vec3(rawData[i * 8 + 0], rawData[i * 8 + 1], rawData[i * 8 + 2]);
        vertex.Normal   = glm::vec3(rawData[i * 8 + 3], rawData[i * 8 + 4], rawData[i * 8 + 5]);
        vertex.TexCoords = glm::vec2(rawData[i * 8 + 6], rawData[i * 8 + 7]);
        vertices.push_back(vertex);
    }
    return vertices;
}

void GenerateSphere(std::vector<graphics::Vertex>& vertices, std::vector<unsigned int>& indices){
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            normals.push_back(glm::vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    vertices.reserve(positions.size());
    for (size_t i = 0; i < positions.size(); ++i)
    {
        vertices.push_back({
            positions[i],
            normals[i],
            uv[i]
        });
    }
}

}


