#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // glm::radians

namespace graphics {

enum class LightType {
    Directional,
    Point,
    Spot
};

class Light {
public:


    explicit Light(LightType type) : m_Type(type) {}
    virtual ~Light() = default;

    LightType GetType() const { return m_Type; }

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
    LightType m_Type;
    glm::vec3 m_Color{1.0f};   ///< 默认白光
    float m_Intensity{1.0f};   ///< 强度（乘在颜色上）
    bool m_Enabled{true};
};

// ==========================================
// 方向光
class DirectionalLight : public Light {
public:
    DirectionalLight() : Light(LightType::Directional) {}

    const glm::vec3& GetDirection() const override { return m_Direction; }
    void SetDirection(const glm::vec3& dir) override { m_Direction = glm::normalize(dir); }
    glm::mat4 GetLightVP(const float aspectRatio = 1.0f, const float nearPlane = 1.0f, float farPlane = 7.5f) const {
        // 假设正交投影，视图矩阵为单位矩阵
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
        glm::mat4 lightView = glm::lookAt(-normalize(m_Direction) * 10.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        return lightProjection * lightView;
    }

private:
    glm::vec3 m_Direction = glm::vec3(-0.2f, -1.0f, -0.3f); // 默认方向
};

// ==========================================
// 点光源
class PointLight : public Light {
public:
    PointLight() : Light(LightType::Point) {}

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
    // 获取立方体贴图的VP矩阵
    std::vector<glm::mat4> GetLightVP(const float aspectRatio = 1.0f, const float nearPlane = 1.0f, const float farPlane = 7.5f) const {
        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspectRatio, nearPlane, farPlane);
        std::vector<glm::mat4> lightViews(6);
        // 六个方向的视图矩阵
        lightViews[0] = glm::lookAt(m_Position, m_Position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // +X
        lightViews[1] = glm::lookAt(m_Position, m_Position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // -X
        lightViews[2] = glm::lookAt(m_Position, m_Position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // +Y
        lightViews[3] = glm::lookAt(m_Position, m_Position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // -Y
        lightViews[4] = glm::lookAt(m_Position, m_Position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // +Z
        lightViews[5] = glm::lookAt(m_Position, m_Position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // -Z
        for (auto& view : lightViews) {
            view = lightProjection * view; // 组合投影和视图矩阵
        }
        return lightViews;
    }

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
    SpotLight() : Light(LightType::Spot) {}

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
    // 获取聚光灯的VP矩阵
    glm::mat4 GetLightVP(const float aspectRatio = 1.0f, const float nearPlane = 1.0f, const float farPlane = 7.5f) const {
        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), aspectRatio, nearPlane, farPlane);
        glm::mat4 lightView = glm::lookAt(m_Position, m_Position + m_Direction, glm::vec3(0.0f, 1.0f, 0.0f));
        return lightProjection * lightView;
    }

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
