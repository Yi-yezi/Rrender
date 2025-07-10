#pragma once

#include <glm/glm.hpp>

namespace graphics {

class Light {
public:
    enum class Type {
        Directional,
        Point,
        Spot
    };

    explicit Light(Type type) : m_Type(type) {}
    virtual ~Light() = default;

    Type GetType() const { return m_Type; }

    void SetColor(const glm::vec3& color) { m_Color = color; }
    const glm::vec3& GetColor() const { return m_Color; }

    void SetIntensity(float intensity) { m_Intensity = intensity; }
    float GetIntensity() const { return m_Intensity; }

private:
    Type m_Type;
    glm::vec3 m_Color{1.0f};   ///< 默认白光
    float m_Intensity{1.0f};   ///< 强度（乘在颜色上）
};

// ==========================================
// 方向光：只有方向和颜色，没有位置和衰减
class DirectionalLight : public Light {
public:
    DirectionalLight() : Light(Type::Directional) {}

    void SetDirection(const glm::vec3& dir) { m_Direction = glm::normalize(dir); }
    const glm::vec3& GetDirection() const { return m_Direction; }

private:
    glm::vec3 m_Direction = glm::vec3(-0.2f, -1.0f, -0.3f); // 默认光照方向
};

// ==========================================
// 点光源：有位置和衰减，无方向
class PointLight : public Light {
public:
    PointLight() : Light(Type::Point) {}

    void SetPosition(const glm::vec3& pos) { m_Position = pos; }
    const glm::vec3& GetPosition() const { return m_Position; }

    void SetAttenuation(float constant, float linear, float quadratic) {
        m_Constant = constant;
        m_Linear = linear;
        m_Quadratic = quadratic;
    }

    float GetConstant() const { return m_Constant; }
    float GetLinear() const { return m_Linear; }
    float GetQuadratic() const { return m_Quadratic; }

private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    float m_Constant = 1.0f;
    float m_Linear = 0.09f;
    float m_Quadratic = 0.032f;
};

// ==========================================
// 聚光灯：结合点光源 + 方向 + 内外角
class SpotLight : public Light {
public:
    SpotLight() : Light(Type::Spot) {}

    void SetPosition(const glm::vec3& pos) { m_Position = pos; }
    const glm::vec3& GetPosition() const { return m_Position; }

    void SetDirection(const glm::vec3& dir) { m_Direction = glm::normalize(dir); }
    const glm::vec3& GetDirection() const { return m_Direction; }

    void SetCutOff(float inner, float outer) {
        m_InnerCutOff = inner;
        m_OuterCutOff = outer;
    }

    float GetInnerCutOff() const { return m_InnerCutOff; }
    float GetOuterCutOff() const { return m_OuterCutOff; }

    void SetAttenuation(float constant, float linear, float quadratic) {
        m_Constant = constant;
        m_Linear = linear;
        m_Quadratic = quadratic;
    }

    float GetConstant() const { return m_Constant; }
    float GetLinear() const { return m_Linear; }
    float GetQuadratic() const { return m_Quadratic; }

private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    glm::vec3 m_Direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float m_InnerCutOff = glm::cos(glm::radians(12.5f));
    float m_OuterCutOff = glm::cos(glm::radians(17.5f));

    float m_Constant = 1.0f;
    float m_Linear = 0.09f;
    float m_Quadratic = 0.032f;
};

} // namespace graphics
