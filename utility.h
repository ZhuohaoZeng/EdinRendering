#pragma once
#include <cmath>
#include <cstdlib>

constexpr double pi = 3.14159265358979323846;
constexpr double invPi = 1.0 / 3.14159265358979323846;
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}