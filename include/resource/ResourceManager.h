#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Model.h"

namespace core {

class ResourceManager {
public:
    static std::shared_ptr<graphics::Shader> LoadShader(const std::string& vertexPath, const std::string& fragmentPath);
    static std::shared_ptr<graphics::Shader> GetShader(const std::string& name);

    static std::shared_ptr<graphics::Texture> LoadTexture(const std::string& texturePath);
    static std::shared_ptr<graphics::Texture> GetTexture(const std::string& name);

    static std::shared_ptr<graphics::Model> LoadModel(const std::string& modelPath);
    static std::shared_ptr<graphics::Model> GetModel(const std::string& name);

private:
    static std::string ExtractName(const std::string& path);

    static std::unordered_map<std::string, std::shared_ptr<graphics::Shader>> m_Shaders;
    static std::unordered_map<std::string, std::shared_ptr<graphics::Texture>> m_Textures;
    static std::unordered_map<std::string, std::shared_ptr<graphics::Model>> m_Models;
};

} // namespace core
