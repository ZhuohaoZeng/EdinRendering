#pragma once
#include "vec3.h"
#include "color.h"
#include "hittable.h"
#include <vector>

struct PointLightRT {
    point3 pos;
    color  intensity;
    PointLightRT(point3 p, color I) : pos(p), intensity(I) {}
    PointLightRT(point3 p, double power_w)             // 允许直接用 Blender 的功率
        : pos(p), intensity( (power_w / (4.0 * pi)) * color(1,1,1) ) {}
};

static constexpr double EPS = 1e-4;

inline color BlinnPhongDiffuse(
    const hit_record& rec,
    const hittable& world,
    const std::vector<PointLightRT>& lights
){
    color Lo(0,0,0);
    const vec3 n = rec.normal;

    for (const auto& L : lights) {
        vec3  toL  = L.pos - rec.p;
        double dist = toL.length();
        vec3  wi   = toL / dist;

        // 阴影检测：长度限制在 dist
        hit_record tmp;
        ray shadow(rec.p + EPS * wi, wi);
        if (!world.hit(shadow, interval(EPS, dist - EPS), tmp)) {
            double ndotl = std::max(0.0, dot(n, wi));
            color Li = L.intensity / (dist * dist);
            Lo += ndotl * Li;                      
        }
    }
    return Lo;
}

inline color BlinnPhongSpec(
    const hit_record& rec, const hittable& world,
    const std::vector<PointLightRT>& lights, const vec3& wo,
    const color& ks, double shininess
){
    color Ls(0);
    const vec3 n = rec.normal;

    for (const auto& L : lights) {
        vec3 toL = L.pos - rec.p;
        double dist = toL.length();
        vec3 wi = toL / dist;

        hit_record tmp;
        if (!world.hit(ray(rec.p + EPS*wi, wi), interval(EPS, dist-EPS), tmp)) {
            double ndotl = std::max(0.0, dot(n, wi));
            if (ndotl <= 0.0) continue;

            vec3  h = unit_vector(wi + wo);
            double ndoth = std::max(0.0, dot(n, h));
            color Li = L.intensity / (dist*dist);

            // 经典做法常乘 ndotl 抑制“雾感”（可留可去）
            Ls += ks * std::pow(ndoth, shininess) * Li;
        }
    }
    return Ls;
}
