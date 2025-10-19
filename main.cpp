#include <fstream>
#include <iostream>
#include <string>
#include "JSONReader.h"
#include "ray.h"
#include "camera.h"
#include "hittable_list.h"
int main() {
    using clock = std::chrono::steady_clock;
    JSONReader reader{};
    bd::Scene scene = reader.loadFromFile("D:/Edin/rendering/scene_export.json");
    hittable_list objects;
    objects.loadScene(scene);
    rt::camera mainCamera(scene.cameras[0]);

    auto t0 = clock::now();
    objects.buildBVH();
    mainCamera.render(objects);
    auto t1 = clock::now();

    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto us  = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds >(t1 - t0).count();
    std::clog << "With BVH - ";
    std::clog << "Elapsed: " << ms << " ms (" << us << " us, " << ns << " ns)\n";
    return 0;
}

