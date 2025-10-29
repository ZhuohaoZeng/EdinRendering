#pragma once
#include "color.h"
#include <iostream>
#include "scene.h"
#include "mat3.h"
#include "utility.h"
#include "material.h"
#include "lighting.h"
namespace rt {// i.e. ray tracing
class camera {
public:
    int  samples_per_pixel = 100;
    int    max_depth         =6;   // Maximum number of ray bounces into scene

    camera(bd::Camera& camera_data)
    {
        image_width = camera_data.film_x;
        aspect_ratio = double(camera_data.film_x) / camera_data.film_y;
        center = vec3{ camera_data.location.x(), camera_data.location.y(), camera_data.location.z() };
        gaze   = vec3{ camera_data.gaze.x(), camera_data.gaze.y(), camera_data.gaze.z() };
        up     = vec3{ camera_data.up.x(), camera_data.up.y(), camera_data.up.z()};
        focal_mm    = camera_data.focal_mm;   
        sensor_w_mm = camera_data.sensor_w_mm;
        sensor_h_mm = camera_data.sensor_h_mm;
        focus_dist  = 1.0;
    }

    void render(const hittable& objects, const std::vector<PointLightRT>& pl)
    {
        initialize();
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
            for (int j = 0; j < image_height; ++j) {
                std::clog << "\rScanline: " << (j + 1) << " / " << image_height << std::endl;
                for (int i = 0; i < image_width; ++i) {
                    color pixelColor(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixelColor += ray_color(r, max_depth, objects, pl);
                    }
                    writeColor(std::cout, pixel_samples_scale * pixelColor);
                }
            }
            std::clog << "\rDone.                 \n";
    }
private: 
    double aspect_ratio;
    int    image_width;
    int    image_height;
    double focal_mm;
    double sensor_w_mm;
    double sensor_h_mm;
    point3 center;
    vec3   gaze;
    vec3   up;
    double focus_dist;
    vec3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;


    
    double pixel_samples_scale;

    std::vector<bd::PointLight> pointLights;

    void initialize() 
    {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        vec3 fwd = unit_vector(gaze);
        up = unit_vector(up);
        vec3 u = unit_vector(cross(fwd, up));
        vec3 v = cross(u, fwd);
        
        double vfov = 2.0 * std::atan((sensor_h_mm * 0.5) / focal_mm); // radians
        double viewport_height = 2.0 * focus_dist * std::tan(vfov * 0.5);
        double viewport_width  = viewport_height * (double(image_width) / image_height);
        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = -viewport_height * v;

        pixel_delta_u = viewport_u / double(image_width);
        pixel_delta_v = viewport_v / double(image_height);
        point3 viewport_upper_left = center + fwd*focus_dist - viewport_u*0.5 - viewport_v*0.5;
        pixel00_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);

        //Antialiasing
        pixel_samples_scale = 1.0 / samples_per_pixel;
    }

    color ray_color_normal(const ray& r, const hittable& objects) const 
    {
        hit_record rec;
        if (objects.hit(r, interval(1e-4, infinity), rec)) {
            return 0.5 * (rec.normal + color(1,1,1));
        }
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

    color ray_color(const ray& r, int depth, const hittable& objects) const //use for ray tracing without caring about the light
    {
        
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;
        if (objects.hit(r, interval(0.001, infinity), rec)) {
            if (!rec.mat) 
            {                       // 关键：检查材质指针
            std::clog << "ERROR: rec.mat is null\n";
            }
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1, objects);
            return color(0,0,0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

    color ray_color(const ray& r, int depth, const hittable& world,
                const std::vector<PointLightRT>& lights) const 
    {
        if (depth <= 0) return color(0,0,0);

        hit_record rec;
        if (!world.hit(r, interval(0.001, infinity), rec)) {
            return color(0.5,0.7,1.0);
        }

        vec3 wo = -unit_vector(r.direction());

        if (auto L = std::dynamic_pointer_cast<const lambertian>(rec.mat)) {
            color kd = L->get_albedo(rec);
            color Ld = BlinnPhongDiffuse(rec, world, lights);
            color Ls = BlinnPhongSpec(rec, world, lights, wo, /*ks=*/color(0.6), /*shininess=*/24.0);
            return kd * Ld + Ls;               // 漫反 * 直射 + 高光
        }

        // if (auto M = dynamic_cast<const metal*>(rec.mat)) {
        //     vec3 refl = reflect(wo, rec.normal);       // 完美镜面反射
        //     ray  rr(rec.p + EPS * refl, refl);
        //     color Lr = ray_color(rr, depth - 1, world, lights);
        //     // 你可以选择是否用金属 albedo 给反射上色
        //     return /*M->get_albedo() * */ Lr;
        // }

        if (auto D = std::dynamic_pointer_cast<idealDielectric>(rec.mat)) {  // 若你用 raw* 就用 dynamic_cast
            const double ior = D->get_ior();
            const double eta = rec.front_face ? (1.0 / ior) : ior;

            // wo：从表面指向相机的方向
            vec3 wi  = unit_vector(r.direction());
            // cosθ 用 wo 与法线的夹角（注意都要单位向量）
            double cos_theta = std::fmin(dot(-wi, rec.normal), 1.0);
            double sin2_theta = 1.0 - cos_theta * cos_theta;

            // 施里克菲涅耳（基于材料 ior）
            double F = reflectance(cos_theta, ior);

            // 反射方向（总是可用）
            vec3 refl_dir = reflect(wi, rec.normal);
            color Lr = ray_color(ray(rec.p + EPS * refl_dir, refl_dir), depth - 1, world, lights);

            color Lt(0,0,0);
            // 先判断是否全内反射
            if (eta * eta * sin2_theta <= 1.0) {
                // 非 TIR 才计算折射
                vec3 refr_dir = refract(wi, rec.normal, eta);  // 你的 3 参数版本
                Lt = ray_color(ray(rec.p + EPS * refr_dir, refr_dir), depth - 1, world, lights);
            } else {
                // 全内反射：全部走反射
                F = 1.0;
            }
            color Ls = BlinnPhongSpec(rec, world, lights, /*wo=*/-wi, color(0.6), 24.0);
            return Ls + F * Lr + (1.0 - F) * Lt;
        }
        return BlinnPhongDiffuse(rec, world, lights);
    }

    ray get_ray(int i, int j) const {
        vec3 offset = sample_square();
        point3 pixel_center = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);
        vec3 ray_direction = pixel_center - center;
        return ray{center, ray_direction};
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }
};
}