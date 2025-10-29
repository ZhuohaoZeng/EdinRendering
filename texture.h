#pragma once
#include "color.h"
#include <vector>


struct Texture {
    virtual color sample(point2 uv) const = 0;
    virtual color sample(double u, double v) const = 0;
    virtual ~Texture() = default;
};

struct ImageTexture : Texture {
    ImageTexture(std::string address)
    {
        std::string abs{"D:/Edin/rendering/scene_export2/"};

    }
    int w,h; std::vector<color> pixels; // 从 .ppm 读入线性空间
    color sample(double u, double v) const override {
        u = u - floor(u); v = v - floor(v);  // wrap
        int i = std::clamp(int(u*w), 0, w-1);
        int j = std::clamp(int((1.0 - v)*h), 0, h-1); // v 向上
        return pixels[j*w + i];
    }
    color sample(point2 uv) const override{
        return sample(uv.x(), uv.y());
    }
};
