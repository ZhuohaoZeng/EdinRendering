#include "JSONReader.h"
#include <fstream>
using json = nlohmann::json;

static vec3 makeVec3(const json& j)
{
    return {j.at(0).get<double>(), j.at(1).get<double>(), j.at(2).get<double>()};
}

JSONReader::JSONReader() { registerDefaultHandlers(); }

void JSONReader::registerHandler(const std::string& section, SectionHandler handler)
{
    handlers_[section] = std::move(handler);
}

Scene JSONReader::loadFromFile(const std::string& filename) const
{
    std::ifstream ifs(filename);
    if (!ifs) throw std::runtime_error("Cannot open JSON: " + filename);
    json j; ifs >> j;
    return load(j);
}

Scene JSONReader::load(const nlohmann::json& j) const
{
    Scene scene{};

    //TODO: META scene data load.

    for (const auto& [key, handler] : handlers_) 
    {
        if (j.contains(key)) handler(j.at(key), scene);
    }
    return scene;
}

void JSONReader::registerDefaultHandlers()
{
    registerHandler("cameras", [](const json& arr, Scene& scene)
    {
        if (!arr.is_array()) return;
        for (const json& camj : arr) {
            Camera c{};
            if (camj.contains("name")) c.name = camj["name"].get<std::string>();
            if (camj.contains("location")) c.location = makeVec3(camj["location"]);
            if (camj.contains("gaze"))     c.gaze     = makeVec3(camj["gaze"]);
            if (camj.contains("up"))       c.up       = makeVec3(camj["up"]);
            if (camj.contains("focal_length_mm"))  c.focal_mm   = camj["focal_length_mm"].get<double>();
            if (camj.contains("sensor_width_mm"))  c.sensor_w_mm = camj["sensor_width_mm"].get<double>();
            if (camj.contains("sensor_height_mm")) c.sensor_h_mm = camj["sensor_height_mm"].get<double>();
            if (camj.contains("film_resolution")) {
                const auto& fr = camj["film_resolution"];
                if (fr.contains("x")) c.film_x = fr["x"].get<int>();
                if (fr.contains("y")) c.film_y = fr["y"].get<int>();
            }
            scene.cameras.push_back(std::move(c));
        }
    });

    registerHandler("point_lights", [](const json& arr, Scene& scene) 
    {
        if (!arr.is_array()) return;
        for (const json& lj : arr) {
            PointLight l;
            if (lj.contains("name")) l.name = lj["name"].get<std::string>();
            if (lj.contains("location")) l.location = makeVec3(lj["location"]);
            if (lj.contains("radiant_intensity")) l.radiant_intensity = lj["radiant_intensity"].get<double>();
            scene.point_lights.push_back(std::move(l));
        }
    });

    registerHandler("spheres", [](const json& arr, Scene& scene) {
        if (!arr.is_array()) return;
        for (const auto& sj : arr) {
            Sphere s;
            if (sj.contains("name")) s.name = sj["name"].get<std::string>();
            if (sj.contains("location")) s.location = makeVec3(sj["location"]);
            if (sj.contains("radius")) s.radius = sj["radius"].get<double>();
            scene.spheres.push_back(std::move(s));
        }
    });

    registerHandler("cubes", [](const json& arr, Scene& scene) {
        if (!arr.is_array()) return;
        for (const auto& cj : arr) {
            Cube c;
            if (cj.contains("name")) c.name = cj["name"].get<std::string>();
            if (cj.contains("translation")) c.translation = makeVec3(cj["translation"]);
            if (cj.contains("rotation_euler_xyz_radians")) c.rotation_euler_xyz_rad = makeVec3(cj["rotation_euler_xyz_radians"]);
            if (cj.contains("scale_1d")) c.scale_1d = cj["scale_1d"].get<double>();
            scene.cubes.push_back(std::move(c));
        }
    });
    
    registerHandler("planes", [](const json& arr, Scene& scene) {
        if (!arr.is_array()) return;
        for (const auto& pj : arr) {
            Plane p; p.corners.resize(4);
            if (pj.contains("name")) p.name = pj["name"].get<std::string>();    
            if (pj.contains("corners") && pj["corners"].is_array() && pj["corners"].size()==4) {
                for (int i=0;i<4;++i) p.corners[i] = makeVec3(pj["corners"][i]);
            }
            if (pj.contains("texture")) p.texture = pj["texture"].get<std::string>();
            if (pj.contains("uv_scale") && pj["uv_scale"].is_array() && pj["uv_scale"].size()>=2)
                p.uv_scale = vec2(pj["uv_scale"][0].get<double>(), pj["uv_scale"][1].get<double>());
            if (pj.contains("uv_offset") && pj["uv_offset"].is_array() && pj["uv_offset"].size()>=2)
                p.uv_offset = vec2(pj["uv_offset"][0].get<double>(), pj["uv_offset"][1].get<double>());
            scene.planes.push_back(std::move(p));
        }
    });
    //TODO: Add support to more shapes or objects form blender
}