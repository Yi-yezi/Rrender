#pragma once
#include <string>
#include "project_root_config.h"

class PathResolver {
public:
    static std::string Resolve(const std::string& relativePath) {
        return std::string(PROJECT_ROOT) + "/" + relativePath;
    }
};
