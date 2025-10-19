#pragma once
#include "hittable.h"
#include "vec3.h"
class triangle : public hittable {
    
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        const double EPS = 1e-9;
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        vec3 p  = cross(r.direction(), e2);
        double det = dot(e1, p);

        // 双面：用绝对值判断是否近平行
        if (std::fabs(det) < EPS) return false;
        double invDet = 1.0 / det;

        vec3 tvec = r.origin() - v0;
        double u = dot(tvec, p) * invDet;
        if (u < 0.0 || u > 1.0) return false;

        vec3 q = cross(tvec, e1);
        double v = dot(r.direction(), q) * invDet;
        if (v < 0.0 || u + v > 1.0) return false;

        double t = dot(e2, q) * invDet;
        if (t < 0) return false;

        rec.t = t;
        rec.p = r.at(t);
        rec.set_face_normal(r, unit_vector(cross(e1, e2))); // 朝向由 (v0,v1,v2) 的绕序决定
        return true;
    }
private:
    point3 v0{}, v1{}, v2{};
};