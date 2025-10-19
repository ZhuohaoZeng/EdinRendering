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

public:
    cube(vec3 translation, vec3 rotation_euler_xyz_rad, double scale_1d) 
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
        planes.emplace_back(P[0], P[1], P[2], P[3]);// -Z (back):   (0,1,2,3)
        planes.emplace_back(P[4], P[5], P[6], P[7]);// +Z (front):  (4,5,6,7)
        planes.emplace_back(P[0], P[1], P[5], P[4]);// -Y (bottom): (0,1,5,4)
        planes.emplace_back(P[3], P[2], P[6], P[7]);// +Y (top):    (3,2,6,7)
        planes.emplace_back(P[0], P[4], P[7], P[3]);// -X (left):   (0,4,7,3)
        planes.emplace_back(P[1], P[2], P[6], P[5]);// +X (right):  (1,2,6,5)
    }

    bounds3 getBounds() const override 
    {
        //TODO
        return bounds3();
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
            }
        }
        return hit_any;
    }
    };

}
