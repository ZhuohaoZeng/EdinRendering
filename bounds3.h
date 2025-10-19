#pragma once
#include "vec3.h"
#include "ray.h"
#include "interval.h"
class bounds3
{
public:
    point3 pMin, pMax;
    bounds3()
    {
        const double INF = std::numeric_limits<double>::infinity();
        pMin = point3( INF,  INF,  INF);
        pMax = point3(-INF, -INF, -INF);
    }
    bounds3(const point3 p) : pMin(p), pMax(p){}
    bounds3(const point3 p1, const point3 p2)
    {
        pMin = point3(fmin(p1.x(), p2.x()), fmin(p1.y(), p2.y()), fmin(p1.z(), p2.z()));
        pMax = point3(fmax(p1.x(), p2.x()), fmax(p1.y(), p2.y()), fmax(p1.z(), p2.z()));
    }
    
    vec3 diagonal() const { return pMax - pMin; }
    
    int maxExtent() const 
    {
        vec3 d = diagonal();
        return (d.x() > d.y() && d.x() > d.z()) ? 0 : d.y() > d.z() ? 1 : 2;
    }
    
    //Might be useful when utilizing USAH I think?
    double surface_area() const 
    {
        vec3 d = diagonal();
        return 2.0 * (d.x()*d.y() + d.x()*d.z() + d.y()*d.z());
    }

    vec3 Centroid() const { return 0.5 * pMin + 0.5 * pMax; }
    
    bounds3 Intersect(const bounds3& b) const
    {
        return bounds3(point3(fmax(pMin.x(), b.pMin.x()), 
                             fmax(pMin.y(), b.pMin.y()), 
                             fmax(pMin.z(), b.pMin.z())),
                       point3(fmin(pMax.x(), b.pMax.x()), 
                             fmin(pMax.y(), b.pMax.y()), 
                             fmin(pMax.z(), b.pMax.z())));
    }

    //map the point to [0,1]^3
    vec3 offset(const point3& p) const 
    {
        vec3 o = p - pMin;
        vec3 d = diagonal();
        if (d.x() > 0) o[0] /= d.x();
        if (d.y() > 0) o[1] /= d.y();
        if (d.z() > 0) o[2] /= d.z();
        return o;
    }

    static bool overlaps(const bounds3& a, const bounds3& b) 
    {
        bool x = (a.pMax.x() >= b.pMin.x()) && (a.pMin.x() <= b.pMax.x());
        bool y = (a.pMax.y() >= b.pMin.y()) && (a.pMin.y() <= b.pMax.y());
        bool z = (a.pMax.z() >= b.pMin.z()) && (a.pMin.z() <= b.pMax.z());
        return x && y && z;
    }

    static bool inside(const point3& p, const bounds3& b) 
    {
        return (p.x() >= b.pMin.x() && p.x() <= b.pMax.x() &&
                p.y() >= b.pMin.y() && p.y() <= b.pMax.y() &&
                p.z() >= b.pMin.z() && p.z() <= b.pMax.z());
    }

    inline const point3& operator[](int i) const { return (i == 0) ? pMin : pMax; }
    inline point3&       operator[](int i)       { return (i == 0) ? pMin : pMax; }

    inline bool intersectP(const ray& r, const vec3& invDir, const interval& ray_t) const
    {

        vec3 t0v = (pMin - r.origin()) * invDir;
        vec3 t1v = (pMax - r.origin()) * invDir;

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
        return t_enter <= t_exit;
    }  
};

inline bounds3 Union(const bounds3& a, const bounds3& b) 
{
    return bounds3(
        point3(std::min(a.pMin.x(), b.pMin.x()),
               std::min(a.pMin.y(), b.pMin.y()),
               std::min(a.pMin.z(), b.pMin.z())),
        point3(std::max(a.pMax.x(), b.pMax.x()),
               std::max(a.pMax.y(), b.pMax.y()),
               std::max(a.pMax.z(), b.pMax.z()))
    );
}

inline bounds3 Union(const bounds3& b, const point3& p) 
{
    return bounds3(
        point3(std::min(b.pMin.x(), p.x()),
               std::min(b.pMin.y(), p.y()),
               std::min(b.pMin.z(), p.z())),
        point3(std::max(b.pMax.x(), p.x()),
               std::max(b.pMax.y(), p.y()),
               std::max(b.pMax.z(), p.z()))
    );
}