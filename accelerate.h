/*
    IN THIS FILE you will find acceleration structures for the ray tracer:

    1) A superclass for tree structures called node
    2) A bvh implementation
    3) A kD-tree implementation
*/

#ifndef ACCELERATE_H
#define ACCELERATE_H

#include "common.h"
#include "hittable.h"
#include <cstddef>

//* BASE NODE
class node : public hittable {
  public:
    node() {}
    virtual ~node() = default;

    static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        rec.stats->record_traversal_step();
        if (!bbox.hit(r, ray_t))
            return false;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;
};

//* BVH NODE
class bvh_node : public node {
  public:
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {}

    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        int axis = random_int(0, 2);

        auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;

        size_t object_span = end - start;
        if (object_span <= 0) {
            throw std::invalid_argument("Whoops, Something broke!");
        } else if (object_span == 1) {
            left = right = objects[start];
        } else if (object_span == 2) {
            left = objects[start];
            right = objects[start + 1];
        } else {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span / 2;
            left = make_shared<bvh_node>(objects, start, mid);
            right = make_shared<bvh_node>(objects, mid, end);
        }

        bbox = aabb(left->bounding_box(), right->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        rec.stats->record_traversal_step();
        if (!bbox.hit(r, ray_t))
            return false;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;
};

//* KD-TREE
class kd_node : public node {
  public:
    kd_node(hittable_list list) : kd_node(list.objects, 0, list.bounding_box()) {}

    kd_node(std::vector<shared_ptr<hittable>>& objects, int depth, const aabb& bounds) {
        bbox = bounds;

        int axis = axis_heuristic(); // Get split axis acording to heuristic

        // std::cout << "depth: " << depth << ", axis: " << axis << ", n_objects: " << objects.size() << std::endl;
        if (objects.size() <= min_primitive_count || depth > max_depth) {
            left = make_shared<hittable_list>(objects);
            right = make_shared<hittable_list>();
            // std::cout << "items: " << objects.size() << std::endl;
        } else {
            auto half = bbox.axis_interval(axis).size() / 2;
            auto midway = bbox.axis_interval(axis).min + half;

            auto left_point = get_left_objects(objects, axis, midway);
            auto right_point = get_right_objects(objects, axis, midway);

            aabb left_bbox, right_bbox;

            if (axis == 0) {
                left_bbox = aabb(interval(bbox.x.min, bbox.x.min + half), bbox.y, bbox.z);
                right_bbox = aabb(interval(bbox.x.min + half, bbox.x.max), bbox.y, bbox.z);
            } else if (axis == 1) {
                left_bbox = aabb(bbox.x, interval(bbox.y.min, bbox.y.min + half), bbox.z);
                right_bbox = aabb(bbox.x, interval(bbox.y.min + half, bbox.y.max), bbox.z);
            } else {
                left_bbox = aabb(bbox.x, bbox.y, interval(bbox.z.min, bbox.z.min + half));
                right_bbox = aabb(bbox.x, bbox.y, interval(bbox.z.min + half, bbox.z.max));
            }
            // std::cout << "midway: " << midway << ", left: " << left_point.size() << ", right: " << right_point.size() << std::endl;

            left = make_shared<kd_node>(left_point, depth+1, left_bbox);
            right = make_shared<kd_node>(right_point, depth+1, right_bbox);
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        rec.stats->record_traversal_step();
        if (!bbox.hit(r, ray_t))
            return false;

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    static const int min_primitive_count = 2;
    static const int max_depth = 25;
    int depth;
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

    // returns longest axis (always split on longest axis)
    int axis_heuristic() {
        if (bbox.x.size() > bbox.y.size() && bbox.x.size() > bbox.z.size())
            return 0;
        if (bbox.y.size() > bbox.z.size())
            return 1;
        return 2;
    }

    std::vector<shared_ptr<hittable>> get_left_objects(const std::vector<shared_ptr<hittable>>& objects, int axis,
                                                       double midway) {
        std::vector<shared_ptr<hittable>> left_objs(0);
        for (size_t i = 0; i < objects.size(); i++) {
            if (objects[i]->bounding_box().axis_interval(axis).min < midway) {
                left_objs.push_back(objects[i]);
            }
        }
        return left_objs;
    }

    std::vector<shared_ptr<hittable>> get_right_objects(const std::vector<shared_ptr<hittable>>& objects, int axis,
                                                       double midway) {
        std::vector<shared_ptr<hittable>> right_objs(0);
        for (size_t i = 0; i < objects.size(); i++) {
            if (objects[i]->bounding_box().axis_interval(axis).max >= midway) {
                right_objs.push_back(objects[i]);
            }
        }
        return right_objs;
    }
};

#endif
