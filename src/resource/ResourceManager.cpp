#include "resource/ResourceManager.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace core {

// 静态成员变量必须在cpp文件中定义初始化
std::unordered_map<std::string, std::shared_ptr<graphics::Shader>> ResourceManager::m_Shaders;
std::unordered_map<std::string, std::shared_ptr<graphics::Texture>> ResourceManager::m_Textures;
std::unordered_map<std::string, std::shared_ptr<graphics::Model>> ResourceManager::m_Models;

std::string ResourceManager::ExtractName(const std::string& path) {
    fs::path p(path);
    return p.stem().string();
}

std::shared_ptr<graphics::Shader> ResourceManager::LoadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string name = ExtractName(vertexPath);
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end())
        return it->second;

    try {
        auto shader = std::make_shared<graphics::Shader>(vertexPath, fragmentPath);
        m_Shaders[name] = shader;
        return shader;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shader: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Shader> ResourceManager::GetShader(const std::string& name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end())
        return it->second;
    return nullptr;
}

std::shared_ptr<graphics::Texture> ResourceManager::LoadTexture(const std::string& texturePath) {
    std::string name = ExtractName(texturePath);
    auto it = m_Textures.find(name);
    if (it != m_Textures.end())
        return it->second;

    try {
        auto texture = std::make_shared<graphics::Texture>(texturePath);
        m_Textures[name] = texture;
        return texture;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Texture> ResourceManager::GetTexture(const std::string& name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end())
        return it->second;
    return nullptr;
}

std::shared_ptr<graphics::Model> ResourceManager::LoadModel(const std::string& modelPath) {
    std::string name = ExtractName(modelPath);
    auto it = m_Models.find(name);
    if (it != m_Models.end())
        return it->second;

    try {
        auto model = std::make_shared<graphics::Model>(modelPath,false);
        m_Models[name] = model;
        return model;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Model> ResourceManager::GetModel(const std::string& name) {
    auto it = m_Models.find(name);
    if (it != m_Models.end())
        return it->second;
    return nullptr;
}

} // namespace core
