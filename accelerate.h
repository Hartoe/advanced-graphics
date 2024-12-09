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

    virtual int axis_heuristic() const {
        return random_int(0, 2);
    }

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

    static bool centre_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().centroid().x() < b->bounding_box().centroid().x();
    }
    static bool centre_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().centroid().y() < b->bounding_box().centroid().y();
    }
    static bool centre_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().centroid().z() < b->bounding_box().centroid().z();
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
        int axis = axis_heuristic();

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
    static const int max_depth = 20;
    int depth;
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

    // returns longest axis (always split on longest axis)
    int axis_heuristic() const override {
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

//* BIH
class bih_node : public node {
    public:
        bih_node(hittable_list list) : bih_node(list.objects, 0, list.objects.size(), list.bounding_box(), 0) {}

        bih_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, aabb bounds, int depth) {
            bbox = bounds;
            int axis = axis_heuristic();
            auto comparator = (axis == 0) ? centre_x_compare : (axis == 1) ? centre_y_compare : centre_z_compare;

            size_t object_span = end - start;
            if (object_span <= 0) {
            throw std::invalid_argument("Whoops, Something broke!");
            } else if (object_span < min_objects || depth > max_depth) {
                std::vector<shared_ptr<hittable>> vec;
                for (int i = start; i < end; i ++)
                    vec.push_back(objects[i]);
                left = make_shared<hittable_list>(vec);
                right = make_shared<hittable_list>();
            } else {
                // Sort objects along the split plane
                std::sort(objects.begin() + start, objects.begin() + end, comparator);
                int bound_points[2];
                get_midway_points(objects, start, end, axis, bound_points);

                // Create updated bounding boxes for the left and right node
                aabb left_bbox;
                aabb right_bbox;
                double min_right, max_left;

                if (bound_points[0] == -1) {
                    // No left children, only on the right
                    min_right = get_minimum_bbox(objects, start, end, axis);
                    right_bbox = aabb(
                        (axis == 0) ? interval(min_right, bbox.x.max) : bbox.x,
                        (axis == 1) ? interval(min_right, bbox.y.max) : bbox.y,
                        (axis == 2) ? interval(min_right, bbox.z.max) : bbox.z
                    );
                    left = make_shared<hittable_list>();
                    right = make_shared<bih_node>(objects, start, end, right_bbox, depth + 1);
                } else if (bound_points[1] == -1) {
                    // No right children, only on the left
                    max_left = get_maximum_bbox(objects, start, end, axis);
                    left_bbox = aabb(
                        (axis == 0) ? interval(bbox.x.min, max_left) : bbox.x,
                        (axis == 1) ? interval(bbox.y.min, max_left) : bbox.y,
                        (axis == 2) ? interval(bbox.z.min, max_left) : bbox.z
                    );
                    left = make_shared<bih_node>(objects, start, end, left_bbox, depth + 1);
                    right = make_shared<hittable_list>();
                } else {
                    max_left = get_maximum_bbox(objects, start, bound_points[1], axis);
                    min_right = get_minimum_bbox(objects, bound_points[1], end, axis);
                    left_bbox = aabb(
                        (axis == 0) ? interval(bbox.x.min, max_left) : bbox.x,
                        (axis == 1) ? interval(bbox.y.min, max_left) : bbox.y,
                        (axis == 2) ? interval(bbox.z.min, max_left) : bbox.z
                    );
                    right_bbox = aabb(
                        (axis == 0) ? interval(min_right, bbox.x.max) : bbox.x,
                        (axis == 1) ? interval(min_right, bbox.y.max) : bbox.y,
                        (axis == 2) ? interval(min_right, bbox.z.max) : bbox.z
                    );
                    left = make_shared<bih_node>(objects, start, bound_points[1], left_bbox, depth + 1);
                    right = make_shared<bih_node>(objects, bound_points[1], end, right_bbox, depth + 1);
                }
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
        aabb bbox;
        shared_ptr<hittable> left;
        shared_ptr<hittable> right;
        const int min_objects = 4;
        const int max_depth = 20;

        void get_midway_points(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, int axis, int* res) {
            double split = bbox.axis_interval(axis).min + (bbox.axis_interval(axis).size() / 2);
            
            auto it = std::find_if(objects.begin() + start, objects.begin() + end, [split, axis](shared_ptr<hittable> obj) {
                return obj->bounding_box().centroid().e[axis] > split;
            });

            if (it >= (objects.begin() + end)) {
                // No object found in right side of split
                res[0] = end-1;
                res[1] = -1;
            }
            else if (it == (objects.begin() + start)) {
                // No object found in left side of split
                res[0] = -1;
                res[1] = start;
            }
            else {
                res[0] = int(it - objects.begin())-1;
                res[1] = int(it - objects.begin());
            }
        }

        double get_minimum_bbox(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, int axis) {
            double min;
            bool set = false;
            
            for (int i = start; i < end; i++){
                auto obj_min = objects[i]->bounding_box().axis_interval(axis).min;

                if (!set)
                {
                    min = obj_min;
                    set = true;
                }

                if (obj_min <= min)
                    min = obj_min;
            }

            return min;
        }
        double get_maximum_bbox(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, int axis) {
            double max;
            bool set = false;
            
            for (int i = start; i < end; i++){
                auto obj_max = objects[i]->bounding_box().axis_interval(axis).max;

                if (!set)
                {
                    max = obj_max;
                    set = true;
                }

                if (obj_max >= max)
                    max = obj_max;
            }

            return max;
        }
};

#endif