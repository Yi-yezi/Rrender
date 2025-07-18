#include "pipeline/PBRPipeline.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "graphics/Light.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <string>
#include <GLFW/glfw3.h>

namespace pipeline {

PBRPipeline::PBRPipeline(std::shared_ptr<graphics::Shader> shader,std::shared_ptr<core::Window> windowPtr)
    : m_Shader(std::move(shader)),
      m_WindowPtr(std::move(windowPtr)) {
        glEnable(GL_DEPTH_TEST);
        m_WindowPtr->GetSize(m_Width, m_Height);
    }

void PBRPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                         const std::shared_ptr<graphics::Camera>& camera) {
    if (!m_Shader || !scene || !camera) return;
    m_WindowPtr->GetSize(m_Width, m_Height);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    m_Shader->Bind();

    // Camera uniforms
    m_Shader->SetUniform("view", camera->GetViewMatrix());
    m_Shader->SetUniform("projection", camera->GetProjectionMatrix());
    m_Shader->SetUniform("camPos", camera->GetPosition());

    // Material base color and AO
    m_Shader->SetUniform("albedo", glm::vec3(0.5f, 0.0f, 0.0f));
    m_Shader->SetUniform("ao", 1.0f);

    // Static light setup
    const int lightCount = 4;
    glm::vec3 lightPositions[lightCount] = {
        {-10.0f,  10.0f, 10.0f},
        { 10.0f,  10.0f, 10.0f},
        {-10.0f, -10.0f, 10.0f},
        { 10.0f, -10.0f, 10.0f}
    };
    glm::vec3 lightColors[lightCount] = {
        {300.0f, 300.0f, 300.0f},
        {300.0f, 300.0f, 300.0f},
        {300.0f, 300.0f, 300.0f},
        {300.0f, 300.0f, 300.0f}
    };

    for (int i = 0; i < lightCount; ++i) {
        m_Shader->SetUniform("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
        m_Shader->SetUniform("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    }

    // Render spheres grid
    const int nrRows = 7, nrColumns = 7;
    const float spacing = 2.5f;
    for (int row = 0; row < nrRows; ++row) {
        float metallic = static_cast<float>(row) / nrRows;
        m_Shader->SetUniform("metallic", metallic);

        for (int col = 0; col < nrColumns; ++col) {
            float roughness = glm::clamp(static_cast<float>(col) / nrColumns, 0.05f, 1.0f);
            m_Shader->SetUniform("roughness", roughness);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(
                (col - nrColumns / 2) * spacing,
                (row - nrRows / 2) * spacing,
                0.0f
            ));
            m_Shader->SetUniform("model", model);

            RenderSphere();
        }
    }

    // Render light spheres
    for (int i = 0; i < lightCount; ++i) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPositions[i]);
        model = glm::scale(model, glm::vec3(0.5f));
        m_Shader->SetUniform("model", model);

        RenderSphere();
    }

    m_Shader->Unbind();
}

void PBRPipeline::RenderSphere() {
    if (m_sphereVAO == 0)
    {
        glGenVertexArrays(1, &m_sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

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
        m_indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);           
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(m_sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 3 + 2) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);        
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));        
    }

    glBindVertexArray(m_sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, m_indexCount, GL_UNSIGNED_INT, 0);
}

} // namespace pipeline
