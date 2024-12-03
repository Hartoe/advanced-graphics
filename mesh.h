#ifndef MESH_H
#define MESH_H

#include "quad.h"
#include "hittable_list.h"

class mesh : public hittable_list {
    public:
        mesh(const std::vector<vec3>& vertices, const std::vector<vec3>& face_indices, shared_ptr<material> mat)
        : vertices(vertices), face_indices(face_indices) {
            for(const vec3& faces : face_indices) {
                auto A = vertices[int(faces.x())];
                auto B = vertices[int(faces.y())];
                auto C = vertices[int(faces.z())];
                add(make_shared<triangle>(A, B-A, C-A, mat));
            }
        }
    private:
        std::vector<vec3> vertices;
        std::vector<vec3> face_indices;
    // vertices (contains the vertices of the mesh), objects 'faces' (contains the triangles)
    /*
        mesh(vertices, face_indices) -> uses the list of vertices together with
                                        indices of the faces to create the objects of triangles.
                                        Hit then becomes -> hit of each object.
                                        Optional make the mesh BVH compatible,
                                        call make_shared<bvh_node> after initializing triangle list
    */
};

#endif