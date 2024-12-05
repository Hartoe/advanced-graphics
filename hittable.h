#ifndef HITTABLE_H
#define HITTABLE_H

#include "common.h"
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

class hittable_list : public hittable {
    public:
        std::vector<shared_ptr<hittable>> objects;

        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); }
        hittable_list(std::vector<shared_ptr<hittable>>& objects) {
            for(const shared_ptr<hittable>& obj : objects) {
                add(obj);
            }
        }

        void clear() { objects.clear(); }

        void add(shared_ptr<hittable> object) {
            objects.push_back(object);
            bbox = aabb(bbox, object->bounding_box());
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            hit_record temp_rec;
            temp_rec.stats = rec.stats;
            bool hit_anything = false;
            auto closest = ray_t.max;

            for (const auto& obj : objects) {
                if (obj->hit(r, interval(ray_t.min, closest), temp_rec)) {
                    hit_anything = true;
                    closest = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hit_anything;
        }

        aabb bounding_box() const override { return bbox; }
    private:
        aabb bbox;
};

#endif
