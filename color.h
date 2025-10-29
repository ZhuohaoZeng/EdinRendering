#pragma once
#include "vec3.h"
#include "interval.h"
using color = vec3;

inline void writeColor(std::ostream& out, const color& c) {
    auto r = c.x();
    auto g = c.y();
    auto b = c.z();

    // Translate the [0,1] component values to the byte range [0,255].
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));

    // Write out the pixel color components.
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

class Image {
public:
    Image(int width, int height, const std::string& filename)
        : width_(width), height_(height), filename_(filename),
          pixels_(width * height, color(0, 0, 0)) {}

    color& at(int x, int y)             { return pixels_[y * width_ + x]; }
    const color& at(int x, int y) const { return pixels_[y * width_ + x]; }

    void writePPM() const {
        std::ofstream out(filename_);
        out << "P3\n" << width_ << ' ' << height_ << "\n255\n";
        for (const auto& c : pixels_) writeColor(out, c);
    }

private:
    int width_, height_;
    std::string filename_;
    std::vector<color> pixels_;
};
