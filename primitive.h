#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "hittable.h"

class quad : public hittable {
    public:
        quad(const point& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
        : Q(Q), u(u), v(v), mat(mat)
        {
            auto n = cross(u, v);
            normal = unit_vector(n);
            D = dot(normal, Q);
            w = n / dot(n, n);

            set_bounding_box();
        }

        virtual void set_bounding_box() {
            auto bbox_diagonal1 = aabb(Q, Q + u + v);
            auto bbox_diagonal2 = aabb(Q + u, Q + v);
            bbox = aabb(bbox_diagonal1, bbox_diagonal2);
        }

        aabb bounding_box() const override { return bbox; }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            rec.stats->record_intersection_test();
            auto denom = dot(normal, r.direction());

            if (std::fabs(denom) < 1e-8)
                return false;

            auto t = (D - dot(normal, r.origin())) / denom;
            if (!ray_t.contains(t))
                return false;

            auto intersection = r.at(t);
            vec3 planar_hitpt_vector = intersection - Q;
            auto alpha = dot(w, cross(planar_hitpt_vector, v));
            auto beta = dot(w, cross(u, planar_hitpt_vector));

            if (!is_interior(alpha, beta, rec))
                return false;

            rec.t = t;
            rec.p = intersection;
            rec.mat = mat;
            rec.set_face_normal(r, normal);

            return true;
        }

        virtual bool is_interior(double a, double b, hit_record& rec) const {
            interval unit_interval = interval(0,1);

            if (!unit_interval.contains(a) || !unit_interval.contains(b))
                return false;
            
            rec.u = a;
            rec.v = b;
            return true;
        }

    private:
        point Q;
        vec3 u, v, w;
        shared_ptr<material> mat;
        aabb bbox;
        vec3 normal;
        double D;
};

class triangle : public quad {
    public:
        triangle(const point& Q, const vec3& u, const vec3& v, shared_ptr<material> mat) : quad(Q, u, v, mat) {}

        bool is_interior(double a, double b, hit_record& rec) const override {
            auto gamma = 1.0 - a - b;
            if (a < 0 || b < 0 || gamma < 0)
                return false;

            rec.u = a;
            rec.v = b;
            return true;
        }
    private:
        point Q;
        vec3 u, v, w;
        shared_ptr<material> mat;
        aabb bbox;
        vec3 normal;
        double D;
};

class sphere : public hittable {
    private:
        point center;
        double radius;
        shared_ptr<material> mat;
        aabb bbox;
    public:
        sphere(const point& center, double radius, shared_ptr<material> mat)
            : center(center), radius(std::fabs(radius)), mat(mat) {
                auto rvec = vec3(radius, radius, radius);
                bbox = aabb(center - rvec, center + rvec);
            }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            vec3 oc = center - r.origin();
            auto a = r.direction().length_squared();
            auto h = dot(r.direction(), oc);
            auto c = oc.length_squared() - radius*radius;

            auto discriminant = h*h - a*c;
            if (discriminant < 0)
                return false;

            auto sqrtd = std::sqrt(discriminant);

            auto root = (h - sqrtd) / a;
            if (!ray_t.surrounds(root))
            {
                root = (h + sqrtd) / a;
                if (!ray_t.surrounds(root))
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat = mat;

            return true;
        }

        aabb bounding_box() const override { return bbox; }
};

#endif
