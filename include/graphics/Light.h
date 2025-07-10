#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // glm::radians

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

    // 颜色和强度是所有光源共有
    void SetColor(const glm::vec3& color) { m_Color = color; }
    const glm::vec3& GetColor() const { return m_Color; }

    void SetIntensity(float intensity) { m_Intensity = intensity; }
    float GetIntensity() const { return m_Intensity; }

    // --------- 虚函数，方便统一访问 ---------
    virtual const glm::vec3& GetDirection() const { static glm::vec3 dummy(0.0f); return dummy; }
    virtual void SetDirection(const glm::vec3&) {}

    virtual const glm::vec3& GetPosition() const { static glm::vec3 dummy(0.0f); return dummy; }
    virtual void SetPosition(const glm::vec3&) {}

    virtual void SetAttenuation(float, float, float) {}
    virtual float GetConstant() const { return 1.0f; }
    virtual float GetLinear() const { return 0.0f; }
    virtual float GetQuadratic() const { return 0.0f; }

    virtual void SetCutOff(float, float) {}
    virtual float GetInnerCutOff() const { return 0.0f; }
    virtual float GetOuterCutOff() const { return 0.0f; }

    // 启用开关（可选）
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }

protected:
    Type m_Type;
    glm::vec3 m_Color{1.0f};   ///< 默认白光
    float m_Intensity{1.0f};   ///< 强度（乘在颜色上）
    bool m_Enabled{true};
};

// ==========================================
// 方向光
class DirectionalLight : public Light {
public:
    DirectionalLight() : Light(Type::Directional) {}

    const glm::vec3& GetDirection() const override { return m_Direction; }
    void SetDirection(const glm::vec3& dir) override { m_Direction = glm::normalize(dir); }

private:
    glm::vec3 m_Direction = glm::vec3(-0.2f, -1.0f, -0.3f); // 默认方向
};

// ==========================================
// 点光源
class PointLight : public Light {
public:
    PointLight() : Light(Type::Point) {}

    const glm::vec3& GetPosition() const override { return m_Position; }
    void SetPosition(const glm::vec3& pos) override { m_Position = pos; }

    void SetAttenuation(float constant, float linear, float quadratic) override {
        m_Constant = constant;
        m_Linear = linear;
        m_Quadratic = quadratic;
    }
    float GetConstant() const override { return m_Constant; }
    float GetLinear() const override { return m_Linear; }
    float GetQuadratic() const override { return m_Quadratic; }

private:
    glm::vec3 m_Position = glm::vec3(0.0f);
    float m_Constant = 1.0f;
    float m_Linear = 0.09f;
    float m_Quadratic = 0.032f;
};

// ==========================================
// 聚光灯
class SpotLight : public Light {
public:
    SpotLight() : Light(Type::Spot) {}

    const glm::vec3& GetPosition() const override { return m_Position; }
    void SetPosition(const glm::vec3& pos) override { m_Position = pos; }

    const glm::vec3& GetDirection() const override { return m_Direction; }
    void SetDirection(const glm::vec3& dir) override { m_Direction = glm::normalize(dir); }

    void SetCutOff(float inner, float outer) override {
        m_InnerCutOff = inner;
        m_OuterCutOff = outer;
    }
    float GetInnerCutOff() const override { return m_InnerCutOff; }
    float GetOuterCutOff() const override { return m_OuterCutOff; }

    void SetAttenuation(float constant, float linear, float quadratic) override {
        m_Constant = constant;
        m_Linear = linear;
        m_Quadratic = quadratic;
    }
    float GetConstant() const override { return m_Constant; }
    float GetLinear() const override { return m_Linear; }
    float GetQuadratic() const override { return m_Quadratic; }

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
