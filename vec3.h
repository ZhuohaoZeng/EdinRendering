#pragma once
#include <array>
#include <cmath>
#include <ostream>
#include <algorithm>
#include "utility.h"

class vec2 {
public:
    // ctors
    constexpr vec2() noexcept : d_{0.f, 0.f} {}
    constexpr vec2(float x, float y) noexcept : d_{x, y} {}

    // accessors
    [[nodiscard]] constexpr float x() const noexcept { return d_[0]; }
    [[nodiscard]] constexpr float y() const noexcept { return d_[1]; }

    // index
    [[nodiscard]] constexpr float  operator[](int i) const noexcept { return d_[static_cast<std::size_t>(i)]; }
    constexpr float&               operator[](int i)       noexcept { return d_[static_cast<std::size_t>(i)]; }

    // unary
    [[nodiscard]] constexpr vec2 operator-() const noexcept { return vec2{-d_[0], -d_[1]}; }

    // compound
    vec2& operator+=(const vec2& v) noexcept { d_[0]+=v[0]; d_[1]+=v[1]; return *this; }
    vec2& operator-=(const vec2& v) noexcept { d_[0]-=v[0]; d_[1]-=v[1]; return *this; }
    vec2& operator*=(float s)      noexcept { d_[0]*=s; d_[1]*=s; return *this; }
    vec2& operator/=(float s)      noexcept { float inv = 1.f/s; return (*this *= inv); }

    // length
    [[nodiscard]] float length_squared() const noexcept { return d_[0]*d_[0] + d_[1]*d_[1]; }
    [[nodiscard]] float length()         const noexcept { return std::sqrt(length_squared()); }

    // print
    friend std::ostream& operator<<(std::ostream& os, const vec2& v) {
        return (os << v.d_[0] << ' ' << v.d_[1]);
    }

private:
    std::array<float, 2> d_;
};

// free ops
inline vec2 operator+(const vec2& a, const vec2& b) noexcept { return {a[0]+b[0], a[1]+b[1]}; }
inline vec2 operator-(const vec2& a, const vec2& b) noexcept { return {a[0]-b[0], a[1]-b[1]}; }
inline vec2 operator*(const vec2& a, const vec2& b) noexcept { return {a[0]*b[0], a[1]*b[1]}; } // elementwise
inline vec2 operator*(float s, const vec2& v)       noexcept { return {s*v[0], s*v[1]}; }
inline vec2 operator*(const vec2& v, float s)       noexcept { return s*v; }
inline vec2 operator/(const vec2& v, float s)       noexcept { return (1.f/s)*v; }

inline float dot(const vec2& a, const vec2& b) noexcept { return a[0]*b[0] + a[1]*b[1]; }

// (No 2D cross returning scalar by default; add if you want: cross(a,b)=a.x*b.y - a.y*b.x)

inline vec2 unit_vector(const vec2& v) noexcept {
    float len2 = v.length_squared();
    if (len2 == 0.f) return v;
    float inv = 1.f / std::sqrt(len2);
    return v * inv;
}

class vec3 {
public:
    // 构造
    constexpr vec3() noexcept : d_{0.0, 0.0, 0.0} {}
    constexpr vec3(double x) noexcept : d_{x, x, x} {}
    constexpr vec3(double x, double y, double z) noexcept : d_{x, y, z} {}

    // 访问器（保留原名）
    [[nodiscard]] constexpr double x() const noexcept { return d_[0]; }
    [[nodiscard]] constexpr double y() const noexcept { return d_[1]; }
    [[nodiscard]] constexpr double z() const noexcept { return d_[2]; }

    // 下标
    [[nodiscard]] constexpr double  operator[](int i) const noexcept { return d_[static_cast<std::size_t>(i)]; }
    constexpr double&               operator[](int i)       noexcept { return d_[static_cast<std::size_t>(i)]; }

    // 一元负号
    [[nodiscard]] constexpr vec3 operator-() const noexcept { return vec3{-d_[0], -d_[1], -d_[2]}; }

    // 复合赋值
    vec3& operator+=(const vec3& v) noexcept {
        for (int i = 0; i < 3; ++i) d_[i] += v.d_[static_cast<std::size_t>(i)];
        return *this;
    }
    vec3& operator-=(const vec3& v) noexcept {
        for (int i = 0; i < 3; ++i) d_[i] -= v.d_[static_cast<std::size_t>(i)];
        return *this;
    }
    vec3& operator*=(double s) noexcept {
        for (double& c : d_) c *= s;
        return *this;
    }
    vec3& operator/=(double s) noexcept {
        const double inv = 1.0 / s;
        return (*this *= inv);
    }

    // 长度
    [[nodiscard]] double length_squared() const noexcept {
        // 写法变化：避免重复展开，减少与常见实现的重合
        double acc = 0.0;
        for (double c : d_) acc += c * c;
        return acc;
    }
    [[nodiscard]] double length() const noexcept { return std::sqrt(length_squared()); }

    // 输出
    friend std::ostream& operator<<(std::ostream& os, const vec3& v) {
        return (os << v.d_[0] << ' ' << v.d_[1] << ' ' << v.d_[2]);
    }

    //---------------newly added 
    static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(double min, double max) {
        return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
    }

    bool near_zero() const {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        return (std::fabs(d_[0]) < s) && (std::fabs(d_[1]) < s) && (std::fabs(d_[2]) < s);
    }
//---------------

private:
    std::array<double, 3> d_;
};

// ====== 保留“旧风格”的自由函数与运算符 ======

inline vec3 operator+(const vec3& a, const vec3& b) noexcept {
    return vec3{a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}
inline vec3 operator-(const vec3& a, const vec3& b) noexcept {
    return vec3{a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
// 逐元素乘（保持原 API）
inline vec3 operator*(const vec3& a, const vec3& b) noexcept {
    return vec3{a[0] * b[0], a[1] * b[1], a[2] * b[2]};
}
// 标量乘除（双边）
inline vec3 operator*(double s, const vec3& v) noexcept { return vec3{s * v[0], s * v[1], s * v[2]}; }
inline vec3 operator*(const vec3& v, double s) noexcept { return s * v; }
inline vec3 operator/(const vec3& v, double s) noexcept { return (1.0 / s) * v; }

// 点积 / 叉积（名称不变）
inline double dot(const vec3& a, const vec3& b) noexcept {
    // 写法变化：不直接展开，便于与常见模板拉开差异
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
inline vec3 cross(const vec3& a, const vec3& b) noexcept {
    return vec3{
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

inline vec3 unit_vector(const vec3& v) noexcept {
    const double len2 = v.length_squared();
    if (len2 == 0.0) return v;  // 防止除以零
    const double inv_len = 1.0 / std::sqrt(len2);
    return v * inv_len;
}

//Newly Added -------------------------------------
inline vec3 random_unit_vector() {
    while (true) {
        auto p = vec3::random(-1,1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    double cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    double k = 1.0 - r_out_perp.length_squared();   // 若 <0 则本不该被调用（TIR）
    // 为安全起见可 clamp 到 [0,1]，但更推荐调用前判断 TIR
    if (k < 0.0) k = 0.0;
    vec3 r_out_parallel = -std::sqrt(k) * n;
    return r_out_perp + r_out_parallel;
}

inline double reflectance(double cosine, double refraction_index) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1. - refraction_index) / (1. + refraction_index);
    r0 = r0*r0;
    return r0 + (1. - r0)*std::pow((1. - cosine), 5.);
}

//For ray interacitons on surface

using point3 = vec3;
using point2 = vec2; //But keeping the third dimension always equal to 0;
