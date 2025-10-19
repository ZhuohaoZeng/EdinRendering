#pragma once

#include <string>
#include <vector>
#include "vec3.h"
namespace bd {
struct Camera {
    std::string name;
    vec3 location{};
    vec3 gaze{};
    vec3 up{};
    double focal_mm = 0;
    double sensor_w_mm = 0;
    double sensor_h_mm = 0;
    int film_x = 0, film_y = 0;
};

struct PointLight {
    std::string name;
    vec3 location{};
    double radiant_intensity = 0.0;
};

struct Sphere {
    std::string name;
    vec3 location{};
    double radius = 1.0;
};

struct Triangle {
    point3 v0, v1, v2;
    vec3   e1, e2;
    vec3   n;
    int    material_id = 0;
};

struct Cube {
    std::string name;
    vec3 translation{};
    vec3 rotation_euler_xyz_rad{};
    double scale_1d = 1.0;
};

struct Plane {
    std::string name;
    // 4 corners, each corner is [x,y,z]
    std::vector<point3> corners{};
    std::vector<Triangle> tris;
};

struct SceneMeta {
    int frame = 0;
    double unit_scale_length = 1.0;
};

struct Cylinder {
    std::string name;
    vec3 location{};
    vec3 rotation_euler_xyz_rad{};
    vec3 scale{};
};

// struct Triangle {
//     int  material_id = 0;
//     point3 v0, v1, v2;
//     vec3 e1, e2, n;
// };

struct Scene {
    SceneMeta meta;
    std::vector<Camera> cameras;
    std::vector<PointLight> point_lights;
    std::vector<Sphere> spheres;
    std::vector<Cube> cubes;
    std::vector<Plane> planes;
};
}