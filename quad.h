#ifndef QUAD_H
#define QUAD_H

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

#endif