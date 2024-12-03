#ifndef KDTREE_H
#define KDTREE_H

#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class kd_node : public hittable {
    public:
        kd_node(hittable_list list) : kd_node(list.objects, 0, list.objects.size(), 0, list.bounding_box()) {}

        kd_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, int depth, const aabb& bounds) {
            bbox = bounds;

            int axis = axis_heuristic(); // Get split axis acording to heuristic

            auto comparator = (axis == 0) ? box_x_compare
                            : (axis == 1) ? box_y_compare
                                          : box_z_compare;

            size_t object_span = end - start;

            if (object_span <= min_primitive_count || depth > max_depth) {
                std::vector<shared_ptr<hittable>> list;
                for (int i = 0; i < object_span; i++)
                    list.push_back(objects[start+i]);
                left = right = make_shared<hittable_list>(list);
            } else {
                std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

                auto mid = get_turning_point(objects, start, end, axis);
                auto half = bbox.axis_interval(axis).size() / 2;

                aabb left_bbox = aabb((axis == 0) ? interval(bbox.x.min, bbox.x.min + half) : bbox.x,
                                      (axis == 1) ? interval(bbox.y.min, bbox.y.min + half) : bbox.y,
                                      (axis == 2) ? interval(bbox.z.min, bbox.z.min + half) : bbox.z);
                aabb right_bbox = aabb((axis == 0) ? interval(bbox.x.min + half, bbox.x.max) : bbox.x,
                                       (axis == 1) ? interval(bbox.y.min + half, bbox.y.max) : bbox.y,
                                       (axis == 2) ? interval(bbox.z.min + half, bbox.z.max) : bbox.z);

                left = make_shared<kd_node>(objects, start, mid, depth+1, left_bbox);
                right = make_shared<kd_node>(objects, mid, end, depth+1, right_bbox);
            }
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            if (!bbox.hit(r, ray_t))
                return false;

            bool hit_left = left->hit(r, ray_t, rec);
            bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

            return hit_left || hit_right;
        }

        aabb bounding_box() const override { return bbox; }
    private:
        static const int min_primitive_count = 10;
        static const int max_depth = 50;
        int depth;
        shared_ptr<hittable> left;
        shared_ptr<hittable> right;
        aabb bbox;
        
        // returns longest axis (always split on longest axis)
        int axis_heuristic() {
            if (bbox.x.size() > bbox.y.size() && bbox.x.size() > bbox.z.size()) return 0;
            if (bbox.y.size() > bbox.z.size()) return 1;
            return 2;
        }

        size_t get_turning_point(const std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end, int axis) {
            // Get midway axis length
            auto midway = bbox.axis_interval(axis).min + bbox.axis_interval(axis).size() / 2;

            // find first element that is bigger than midway turn
            auto elem = std::find_if(std::begin(objects) + start, std::begin(objects) + end, [axis, midway](const shared_ptr<hittable>& obj){
                auto obj_axis = obj->bounding_box().axis_interval(axis);
                return obj_axis.min > midway;
            });
            auto index = elem-objects.begin();
            if (index >= objects.size())
                return end;

            return index;
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
};

#endif