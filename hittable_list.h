#pragma once

#include "hittable.h"
#include <iostream>
#include <vector>
#include <memory>
#include "interval.h"
#include "scene.h"
#include "sphere.h"
#include "plane.h"
#include "cube.h"
#include "BVH.h"
#include "lighting.h"
using std::shared_ptr;

class hittable_list : public hittable
{
public:
    std::vector<shared_ptr<hittable>> objects;
    std::vector<PointLightRT> pointLights;
    hittable_list(){};
    std::unique_ptr<BVH> bvh;
    hittable_list(shared_ptr<hittable> object){add(object);}
    void clear(){objects.clear(); bvh.reset();}
    void add(shared_ptr<hittable> object){
        objects.push_back(std::move(object));
        bvh.reset();
    }
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        if (bvh) {
            return bvh->hit(r, ray_t, rec);
        }
        //
        hit_record best{};          // 保存目前为止最好的命中
        bool       found = false;   // 是否命中过任意物体
        double     t_max = ray_t.max;

        for (const shared_ptr<hittable>& obj : objects) {
            hit_record h{};

            // 发现更近的命中就收紧区间上界，确保最终是“最近命中”
            if (obj->hit(r, interval(ray_t.min, t_max), h)) {
                found = true;
                t_max = h.t;        // 缩小搜索窗口
                best  = h;
            }
        }

        if (found) rec = best;
        return found;
    }
    void loadScene(bd::Scene& scene) 
    {
        
        //temp
        auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
        auto material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
        auto material_left   = std::make_shared<dielectric>(1.00 / 1.33);
        auto idealD   = std::make_shared<idealDielectric>(1.50);
        auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2));

        //end
        if (scene.point_lights.empty()) std::clog << "No point lights\n";
        else for (auto& s : scene.point_lights) 
            pointLights.emplace_back(s.location, s.radiant_intensity);

        if (scene.spheres.empty()) std::clog << "No spheres\n";
        else for (auto& s : scene.spheres)
            add(std::make_shared<rt::sphere>(s.location, s.radius, idealD));

        // cubes
        if (scene.cubes.empty()) std::clog << "No cubes\n";
        else for (auto& c : scene.cubes)
            add(std::make_shared<rt::cube>(c.translation, c.rotation_euler_xyz_rad, c.scale_1d, material_center));

        // planes
        if (scene.planes.empty()) std::clog << "No planes\n";
        else for (auto& p : scene.planes)
            add(std::make_shared<rt::plane>(p.corners[0], p.corners[1], p.corners[2], p.corners[3], material_ground));
            // // Codes for uv mapping
            // if (!p.texture.empty()) {
            //     ImageTexture t{p.texture};
            //     auto textureMat = std::make_shared<lambertian>(t);
            //     add(std::make_shared<rt::plane>(p.corners[0], p.corners[1], p.corners[2], p.corners[3], textureMat));
            // }
            // else add(std::make_shared<rt::plane>(p.corners[0], p.corners[1], p.corners[2], p.corners[3], material_ground));
    }

    void buildBVH()
    {
        std::clog << "Generating BVH...\n\n" << std::endl;
        this->bvh = std::make_unique<BVH>(objects);//BVHAccel::SplitMethod::NAIVE
    }
    bounds3 getBounds() const override
    {
        if (bvh) return bvh->getBounds();
        if (objects.empty()) return bounds3();  // empty/invalid box

        bounds3 b = objects[0]->getBounds();
        for (size_t i = 1; i < objects.size(); ++i)
            b = Union(b, objects[i]->getBounds());
        return b;
    }
};