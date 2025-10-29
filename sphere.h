#pragma once
#include "hittable.h"
#include "vec3.h"
namespace rt {
class sphere : public hittable {
private:
point3 center{};
double radius;
std::shared_ptr<material> mat;

public: 
    sphere(const point3& center, double radius, std::shared_ptr<material> mat) 
    : center(center), radius(std::fmax(0,radius)), mat(mat){}

    bounds3 getBounds() const override 
    {
        vec3 e(radius, radius, radius);
        return bounds3(center - e, center + e);
    }
    
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        const vec3  d = r.direction();
        const vec3  m = r.origin() - center;
        const double a = dot(d, d);                   // d·d
        const double b = dot(m, d);                   // m·d 
        const double c = dot(m, m) - radius * radius; // m·m - R²

        const double disc = b * b + (-a) * c;         // b² - a·c
        if (disc < 0.0) return false;

        const double s = std::sqrt(disc);

        double t = (-b - s) / a;
        if (!ray_t.surrounds(t)) {
            t = (-b + s) / a;
            if (!ray_t.surrounds(t)) return false;
        }

        rec.t = t;
        rec.p = r.at(t);
        rec.mat = mat;
        const vec3 outward = (rec.p - center) / radius;
        rec.set_face_normal(r, outward);
        return true;
    }
};


}