#pragma once
#include "hittable.h"
#include "vec3.h"
namespace rt {
class plane : public hittable {
private:
    point3 v0, v1, v2, v3;
public:
    plane(point3 a, point3 b, point3 c, point3 d) : v0(a), v1(b), v2(c), v3(d) {}
    bounds3 getBounds() const override
    {
        //TODO
        return bounds3();
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        hit_record rec0, rec1;
        bool h0 = hit_tri(v0, v1, v2, r, ray_t, rec0);
        bool h1 = hit_tri(v0, v2, v3, r, ray_t, rec1);

        if (!h0 && !h1) return false;
        // pick closer valid hit
        if (h0 && (!h1 || rec0.t < rec1.t)) 
        { rec = rec0; return true; }
        rec = rec1; 
        return true;
    }

    //MollerTrumbore
    bool hit_tri(const point3& a, const point3& b, const point3& c, const ray& r, interval ray_t, hit_record& rec) const
    {
        const double EPS = 1e-9;
        vec3 e1 = b - a;
        vec3 e2 = c - a;
        vec3 p  = cross(r.direction(), e2);
        double det = dot(e1, p);

        if (std::fabs(det) < EPS) return false; // nearly parallel
        double invDet = 1.0 / det;

        vec3 tvec = r.origin() - a;
        double u = dot(tvec, p) * invDet;
        if (u < 0.0 || u > 1.0) return false;

        vec3 q = cross(tvec, e1);
        double v = dot(r.direction(), q) * invDet;
        if (v < 0.0 || u + v > 1.0) return false;

        double t = dot(e2, q) * invDet;
        if (!ray_t.surrounds(t)) return false;

        rec.t = t;
        rec.p = r.at(t);
        vec3 n = cross(e1, e2);
        rec.set_face_normal(r, unit_vector(n));
        //rec.u = u; rec.v = v;
        //rec.mat = material;
        return true;
    }
};
}