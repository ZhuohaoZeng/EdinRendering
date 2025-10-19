#pragma once
#include "hittable.h"
#include <memory>
#include <vector>

class BVHNode : public hittable
{
public:
    BVHNode() = default;
    BVHNode(std::vector<std::shared_ptr<hittable>>& objs,
            size_t start, size_t end);
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override;
    bounds3 getBounds() const override { return box; }

private:
    std::shared_ptr<hittable> left;
    std::shared_ptr<hittable> right;
    bounds3 box;  
    int axis = 0; 
};

class BVH : public hittable {
public:
    BVH() = default;
    explicit BVH(std::vector<std::shared_ptr<hittable>>& objects)
    {
        if (!objects.empty())
            root = std::make_shared<BVHNode>(objects, 0, objects.size());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!root) return false;
        return root->hit(r, ray_t, rec);
    }

    bounds3 getBounds() const override {
        return root ? root->getBounds() : bounds3();
    }

private:
    std::shared_ptr<BVHNode> root;
};