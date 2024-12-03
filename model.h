#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include "accelerate.h"

#include <cstring>

class model : public hittable {
    public:
        model(const char* path, shared_ptr<material> mat, const char* mode = "brute")
        {
            std::vector<vec3> vertices;
            std::vector<vec3> face_indices;
            loadOBJ(path, vertices, face_indices);

            _mesh = mesh(vertices, face_indices, mat);
            if (strcmp(mode, "bvh") == 0)
                _mesh = hittable_list(make_shared<bvh_node>(_mesh));
            else if (strcmp(mode, "kd") == 0)
                _mesh = hittable_list(make_shared<kd_node>(_mesh));
        }

        aabb bounding_box() const override { return _mesh.bounding_box(); }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            return _mesh.hit(r, ray_t, rec);
        }
    private:
        hittable_list _mesh;

        bool loadOBJ(const char* path, std::vector<vec3>& vertices, std::vector<vec3>& face_indices) {
            FILE* file = fopen(path, "r");
            if (file == NULL)
                return false;

            while (true) {
                char lineHeader[128];
                int res = fscanf(file, "%s", lineHeader);
                if (res == EOF)
                    break;

                if (strcmp(lineHeader, "v") == 0) {
                    double x, y, z;
                    fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
                    vertices.push_back(vec3(x, y, z));
                } else if (strcmp(lineHeader, "f") == 0) {
                    unsigned int v1, v2, v3;
                    fscanf(file, "%d %d %d\n", &v1, &v2, &v3);
                    face_indices.push_back(vec3(v1-1, v2-1, v3-1));
                }
            }
            fclose(file);

            return true;
        }
};

#endif