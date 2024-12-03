#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"
#include "aabb.h"
#include "stat_collector.h"

class material;

class hit_record {
  public:
    point p;
    vec3 normal;
    shared_ptr<material> mat;
    shared_ptr<stat_collector> stats;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

    virtual aabb bounding_box() const = 0;
};

#endif
