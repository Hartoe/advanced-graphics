#ifndef MESH_H
#define MESH_H

#include "primitive.h"

class mesh : public hittable_list {
    public:
        mesh() {}
        mesh(const std::vector<vec3>& vertices, const std::vector<vec3>& face_indices, shared_ptr<material> mat)
        : vertices(vertices), face_indices(face_indices) {
            for(int i = 0; i < face_indices.size(); i++) {
                auto A = vertices[int(face_indices[i].x())];
                auto B = vertices[int(face_indices[i].y())];
                auto C = vertices[int(face_indices[i].z())];
                add(make_shared<triangle>(A, B-A, C-A, mat));
            }
        }
    private:
        std::vector<vec3> vertices;
        std::vector<vec3> face_indices;
};

#endif