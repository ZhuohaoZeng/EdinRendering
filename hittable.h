#pragma once
#include "ray.h"
#include "interval.h"
#include "bounds3.h"
#include <memory>


class material;

class hit_record {
public:
    point3 p;
    vec3 normal;
    double t;
    point2 uv;
    bool front_face;
    std::shared_ptr<material> mat;
    //Ensuring that the normal of the plain is always pointing outside the shape
    void set_face_normal(const ray& r, const vec3& outward_normal)
    {
      front_face = dot(r.direction(), outward_normal) < 0;
      normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual ~hittable() = default;
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
    virtual bounds3 getBounds() const = 0;

};
