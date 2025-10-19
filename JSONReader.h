#pragma once
#include <string>
#include <vector>
#include "vec3.h"
#include <nlohmann/json.hpp>
#include "scene.h"
#include <functional>
#include <unordered_map>

using namespace bd;//I.E. Blender data
class JSONReader {
private:
    using SectionHandler = std::function<void(const nlohmann::json&, Scene&)>;
    std::unordered_map<std::string, SectionHandler> handlers_;
    void registerDefaultHandlers();

public:
    JSONReader();
    Scene loadFromFile(const std::string& filename) const;
    Scene load(const nlohmann::json& j) const;

    void registerHandler(const std::string& section, SectionHandler handler);
};