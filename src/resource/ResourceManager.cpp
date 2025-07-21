#include "resource/ResourceManager.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace core {

std::string ResourceManager::ExtractName(const std::string& path) {
    fs::path p(path);
    return p.stem().string();
}

std::shared_ptr<graphics::Shader> ResourceManager::LoadShader(const std::string& vertexPath, const std::string& fragmentPath,
                                                              const std::string& geometryPath) {
    try {
        auto shader = std::make_shared<graphics::Shader>(vertexPath, fragmentPath, geometryPath);
        return shader;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shader: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Texture> ResourceManager::LoadTexture(const std::string& texturePath,
                                                                bool useSRGB, bool useFloat) {
    try {
        auto texture = std::make_shared<graphics::Texture>(texturePath, useSRGB, useFloat);
        return texture;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Model> ResourceManager::LoadModel(const std::string& modelPath, bool useSRGB) {
    try {
        auto model = std::make_shared<graphics::Model>(modelPath, useSRGB);
        return model;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<graphics::Model> ResourceManager::LoadModel(const std::string& name,
                                                            const std::vector<graphics::Vertex>& vertices,
                                                            const std::string& texturePath,
                                                            bool useSRGB) {
    try {
        auto model = std::make_shared<graphics::Model>(vertices, texturePath, useSRGB);
        return model;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model from vertices: " << e.what() << std::endl;
        return nullptr;
    }
}

} // namespace core
