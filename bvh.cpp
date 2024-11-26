#include "bvh.h"

BVH::BVH(Model scene) : scene(scene) {
    //TODO contruct bvh
}

bool BVH::scene_intersect(const Vec3f &orig, const Vec3f &dir,  Vec3f &hit, Vec3f &N){
    float duck_dist = std::numeric_limits<float>::max();
    for (int i=0; i < scene.nfaces(); i++) {
        float dist_i;
        if (scene.ray_triangle_intersect(i, orig, dir, dist_i) && dist_i < duck_dist) {
            duck_dist = dist_i;
            hit = orig + dir*dist_i;
            N = scene.triangle_normal(i).normalize();
        }
    }
    return duck_dist < 1000;
}
