#include "BVH.h"

BVHNode::BVHNode(std::vector<std::shared_ptr<hittable>>& objs,
            size_t start, size_t end)// int max_leaf = 1
{
    // 1) 统计当前节点的整体 AABB（所有物体并集）
    bounds3 node_box = objs[start]->getBounds();
    for (size_t i = start + 1; i < end; ++i)
        node_box = Union(node_box, objs[i]->getBounds());
    box = node_box;

    // 2) 判断是否做叶子
    const size_t span = end - start;
    if (span <= 1) {
        left  = objs[start];   // 叶子里只挂一个（最简单版本）
        right = nullptr;
        axis  = 0;             // 记录个值即可（调试/统计用）
        return;                // 叶子构建结束
    }

    // 3) 计算“质心 AABB”，用于选择最长轴
    bounds3 centroid_box(objs[start]->getBounds().Centroid());
    for (size_t i = start + 1; i < end; ++i)
        centroid_box = Union(centroid_box, objs[i]->getBounds().Centroid());
    axis = centroid_box.maxExtent();

    // 4) 选择比较器（按质心在 axis 轴上的标量排序）
    auto cmp = (axis == 0)
        ? [](const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
                return a->getBounds().Centroid().x() < b->getBounds().Centroid().x();
            }
        : (axis == 1)
        ? [](const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
                return a->getBounds().Centroid().y() < b->getBounds().Centroid().y();
            }
        : [](const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b){
                return a->getBounds().Centroid().z() < b->getBounds().Centroid().z();
            };
    // 5) 选中位数位置并“局部重排”（不完全排序，线性期望时间）
    const size_t mid = start + span / 2;
    std::nth_element(objs.begin() + start,
                        objs.begin() + mid,
                        objs.begin() + end, cmp);

    // 6) 递归构建左右子树（[start, mid) 和 [mid, end)）
    left  = std::make_shared<BVHNode>(objs, start, mid);
    right = std::make_shared<BVHNode>(objs, mid,   end);

    // 7) （可选冗余）确保当前节点 AABB = 左右并集（通常已等于 node_box）
    //    再做一次 Union 让逻辑更直观，也能防止未来改动时漏更新
    box = Union(left->getBounds(), right->getBounds());
}

static inline bool aabb_entry_t(const bounds3& b, const ray& r,
                                const interval& ray_t, double& t_enter_out)
{
    vec3 invDir(1.0/r.direction().x(),
                1.0/r.direction().y(),
                1.0/r.direction().z());
    vec3 t0v = (b.pMin - r.origin()) * invDir;
    vec3 t1v = (b.pMax - r.origin()) * invDir;

    vec3 tmin(std::min(t0v.x(), t1v.x()),
              std::min(t0v.y(), t1v.y()),
              std::min(t0v.z(), t1v.z()));
    vec3 tmax(std::max(t0v.x(), t1v.x()),
              std::max(t0v.y(), t1v.y()),
              std::max(t0v.z(), t1v.z()));

    double t_enter = std::max(tmin.x(), std::max(tmin.y(), tmin.z()));
    double t_exit  = std::min(tmax.x(), std::min(tmax.y(), tmax.z()));
    t_enter = std::max(t_enter, ray_t.min);
    t_exit  = std::min(t_exit,  ray_t.max);

    t_enter_out = t_enter;
    return (t_enter <= t_exit);
}

bool BVHNode::hit(const ray& r, interval ray_t, hit_record& rec) const
{
    vec3 invDir(1.0/r.direction().x(),
                1.0/r.direction().y(),
                1.0/r.direction().z());
    if (!box.intersectP(r, invDir, ray_t)) return false;

    if (!right) 
    {
        hit_record tmp;
        if (left->hit(r, ray_t, tmp)) { rec = tmp; return true; }
        return false;
    }

    double tL = 0.0, tR = 0.0;
    bool hitLbox = aabb_entry_t(left->getBounds(),  r, ray_t, tL);
    bool hitRbox = aabb_entry_t(right->getBounds(), r, ray_t, tR);

    if (!hitLbox && !hitRbox) return false;
    std::shared_ptr<hittable> firstChild, secondChild;
    if (hitLbox && hitRbox) {
        if (tL <= tR) { firstChild = left;  secondChild = right; }
        else          { firstChild = right; secondChild = left;  }
    } else {
        firstChild  = hitLbox ? left : right;
        secondChild = nullptr;
    }

    hit_record r1;
    bool h1 = firstChild->hit(r, ray_t, r1);
    if (h1) ray_t.max = r1.t;
    if (secondChild) {
        hit_record r2;
        bool h2 = secondChild->hit(r, ray_t, r2);

        // [H] 合并两个结果：返回最近的命中
        if (h1 && h2) { rec = (r1.t < r2.t ? r1 : r2); return true; }
        if (h2)       { rec = r2; return true; }
    }

    if (h1) { rec = r1; return true; }
    return false;
}