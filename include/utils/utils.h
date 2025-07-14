#include "graphics/Mesh.h"

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