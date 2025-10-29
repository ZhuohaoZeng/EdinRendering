#pragma once
#include "hittable.h"
#include "color.h"
#include "texture.h"
class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray& /*r_in*/, const hit_record& /*rec*/, color& /*attenuation*/, ray& /*scattered*/
    ) const {
        return false;
    }
};

class lambertian : public material {
public:
    lambertian(const color& albedo) : albedo(albedo) {}
    lambertian(std::shared_ptr<Texture> tex) : tex(tex) {}
    const color& get_albedo(const hit_record& rec) const { 
        if (tex) return tex->sample(rec.uv);
        return albedo; 
    }
    bool scatter(const ray& , const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_unit_vector();
        if (scatter_direction.near_zero()) scatter_direction = rec.normal;
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
    std::shared_ptr<Texture> tex;
};

//TODO: Unfinished materials, TBD later
class metal : public material {
public:
    metal(const color& albedo) : albedo(albedo) {}
    const color& get_albedo() const { return albedo; }
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}
    double get_ior() const { return refraction_index; }
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction);
        return true;
    }

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;
};

class idealDielectric : public material {
public:
    explicit idealDielectric(double ior) : ior_(ior) {}

    bool scatter(const ray& r_in, const hit_record& rec,
                 color& attenuation, ray& scattered) const override
    {
        attenuation = color(1.0, 1.0, 1.0);   // 经典教材玻璃：无吸收
        const double eps = 1e-4;

        // 规范化入射方向，并让法线朝“入射光的反向”
        vec3 n  = rec.front_face ? rec.normal : -rec.normal;
        vec3 in = unit_vector(r_in.direction());

        // 介质比（空气<->玻璃）
        double etai = rec.front_face ? 1.0 : ior_;
        double etat = rec.front_face ? ior_ : 1.0;
        double eta  = etai / etat;

        // 计算入射角余弦 & 全反射判定
        double cosTheta   = std::min(dot(-in, n), 1.0);
        double sinTheta2  = std::max(0.0, 1.0 - cosTheta * cosTheta);
        bool   cannot_refract = (eta * eta * sinTheta2) > 1.0;   // eta*sinθ > 1

        // Fresnel 反射率（你提供的 reflectance 接口假设外侧介质≈1）
        double Fr = reflectance(cosTheta, ior_);
        if (cannot_refract) Fr = 1.0;

        // 按 Fresnel 概率选择反射/折射方向，并做起点偏移避免自相交
        if (cannot_refract || random_double() < Fr) {
            vec3 dir    = reflect(in, n);
            vec3 origin = rec.p + n * eps;     // 反射：向法线方向偏移
            scattered   = ray(origin, dir);
        } else {
            vec3 dir    = refract(in, n, eta); // 你的 refract 已经做了 clamp
            vec3 origin = rec.p - n * eps;     // 折射：向内侧偏移
            scattered   = ray(origin, dir);
        }
        return true;
    }

    double get_ior() const { return ior_; }

private:
    double ior_; // 1.5 for glass
};