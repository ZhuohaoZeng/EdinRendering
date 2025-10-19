#pragma once
#include "hittable.h"
#include "color.h"
#include <iostream>
#include "scene.h"
#include "mat3.h"
namespace rt {// i.e. ray tracing
class camera {
public:
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

    void render(const hittable& objects)
    {
        initialize();
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
            for (int j = 0; j < image_height; ++j) {
                std::clog << "\rScanline: " << (j + 1) << " / " << image_height << std::endl;
                for (int i = 0; i < image_width; ++i) {
                    point3 pixel_center = pixel00_loc + i * pixel_delta_u + j * pixel_delta_v;
                    vec3 ray_direction = pixel_center - center;
                    ray r{center, ray_direction};
                    color pixelColor{ray_color(r, objects)};
                    writeColor(std::cout, pixelColor);
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

    //temp
    std::vector<bd::Sphere>* spheres;
    std::vector<bd::Cube>* cubes;
    std::vector<bd::Plane>* planes;

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
    }

    color ray_color(const ray& r, const hittable& objects) const 
    {
        hit_record rec;
        if (objects.hit(r, interval(1e-4, infinity), rec)) {
            return 0.5 * (rec.normal + color(1,1,1));
        }
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};
}