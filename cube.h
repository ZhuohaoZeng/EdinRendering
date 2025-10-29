#pragma once
#include "hittable.h"
#include "vec3.h"
#include "mat3.h"
#include "plane.h"
#include <vector>
namespace rt {
class cube : public hittable
{
private:
    std::vector<plane> planes;
    point3 minV;
    point3 maxV;
    std::shared_ptr<material> mat;

public:
    cube(vec3 translation, vec3 rotation_euler_xyz_rad, double scale_1d, std::shared_ptr<material> mat) :mat(mat)
    {
        const double s = scale_1d;
        std::vector<point3> P{point3(-s,-s,-s), point3( s,-s,-s),
                              point3( s, s,-s), point3(-s, s,-s),
                              point3(-s,-s, s), point3( s,-s, s),
                              point3( s, s, s), point3(-s, s, s)};
        mat3 R = eulerXYZ_to_mat3(rotation_euler_xyz_rad);
        // rotate then translate
        auto Xform = [&](point3 p){return translation + R * p; };
        for (auto& p : P) p = Xform(p);
        planes.reserve(6);
        // Faces with CCW winding as seen from OUTSIDE
        planes.emplace_back(P[0], P[1], P[2], P[3], mat);// -Z (back):   (0,1,2,3)
        planes.emplace_back(P[4], P[5], P[6], P[7], mat);// +Z (front):  (4,5,6,7)
        planes.emplace_back(P[0], P[1], P[5], P[4], mat);// -Y (bottom): (0,1,5,4)
        planes.emplace_back(P[3], P[2], P[6], P[7], mat);// +Y (top):    (3,2,6,7)
        planes.emplace_back(P[0], P[4], P[7], P[3], mat);// -X (left):   (0,4,7,3)
        planes.emplace_back(P[1], P[2], P[6], P[5], mat);// +X (right):  (1,2,6,5)
    }

    bounds3 getBounds() const override 
    {
        if (planes.empty()) return bounds3();
        double min_x = planes[0].a().x(), min_y = planes[0].a().y(), min_z = planes[0].a().z();
        double max_x = min_x, max_y = min_y, max_z = min_z;

        auto acc = [&](const point3& p) {
            min_x = std::min(min_x, p.x());  max_x = std::max(max_x, p.x());
            min_y = std::min(min_y, p.y());  max_y = std::max(max_y, p.y());
            min_z = std::min(min_z, p.z());  max_z = std::max(max_z, p.z());
        };

        for (const auto& f : planes) {
            acc(f.a()); acc(f.b()); acc(f.c()); acc(f.d());
        }

        const double eps = 1e-6;
        if (max_x - min_x < eps) { min_x -= eps; max_x += eps; }
        if (max_y - min_y < eps) { min_y -= eps; max_y += eps; }
        if (max_z - min_z < eps) { min_z -= eps; max_z += eps; }

        return bounds3(point3(min_x,min_y,min_z), point3(max_x,max_y,max_z));
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        bool hit_any = false;
        double closest = ray_t.max;
        hit_record temp;
        for (const auto& p : planes)
        {
            if (p.hit(r, {ray_t.min, closest}, temp)) 
            {
                hit_any = true;
                closest = temp.t;
                rec = temp;
                rec.mat = mat;
            }
        }
        return hit_any;
    }
    };

}
