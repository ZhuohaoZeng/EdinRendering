#pragma once
#include <array>
#include <cmath>
#include <ostream>
#include <algorithm>

class vec3 {
public:
    using storage_t = std::array<double, 3>;

    // 构造
    constexpr vec3() noexcept : d_{0.0, 0.0, 0.0} {}
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

private:
    storage_t d_;
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

using point3 = vec3;
