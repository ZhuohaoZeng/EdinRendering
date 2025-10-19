#pragma once
#include "vec3.h"

using color = vec3;

inline void writeColor(std::ostream& out, const color& c) {
    auto to_byte = [](double v) -> unsigned char {
        v = std::clamp(v, 0.0, 1.0);
        return static_cast<unsigned char>(std::lround(v * 255.0));
    };
    unsigned char rgb[3] = { to_byte(c.x()), to_byte(c.y()), to_byte(c.z()) };
    out << static_cast<int>(rgb[0]) << ' '
        << static_cast<int>(rgb[1]) << ' '
        << static_cast<int>(rgb[2]) << '\n';
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
