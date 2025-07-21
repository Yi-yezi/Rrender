#pragma once
#include <string>
#include <memory>
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Model.h"

namespace core {

class ResourceManager {
public:
    // 每次都新建资源，不做缓存
    static std::shared_ptr<graphics::Shader> LoadShader(const std::string& vertexPath, const std::string& fragmentPath,
                                                        const std::string& geometryPath = "");

    static std::shared_ptr<graphics::Texture> LoadTexture(const std::string& texturePath,
                                                          bool useSRGB = false, bool useFloat = false);

    static std::shared_ptr<graphics::Model> LoadModel(const std::string& modelPath, bool useSRGB = false);
    static std::shared_ptr<graphics::Model> LoadModel(const std::string& name, const std::vector<graphics::Vertex>& vertices, 
                                                      const std::string& texturePath,
                                                      bool useSRGB = false);

private:
    static std::string ExtractName(const std::string& path);
};

} // namespace core
