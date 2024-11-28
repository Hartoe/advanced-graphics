#include "bvh.h"
#include "geometry.h"


BVH::BVH(Model scene) : scene(scene), tris(), nodes() {
    //TODO contruct bvh
    for (int i=0; i < scene.nfaces(); i++) {
        Vec3f v0 = scene.point(scene.vert(i, 0));
        Vec3f v1 = scene.point(scene.vert(i, 1));
        Vec3f v2 = scene.point(scene.vert(i, 2));
        Tri t = Tri{v0, v1, v2, (v0 + v1 + v2) * 0.33333f};
        tris.push_back(t);
    }

    // assign all triangles to root node
    BVHNode& root = nodes[0];
    root.leftFirst = 0;
    root.triCount = tris.size();
    updateNodeBounds(0);
    // subdivide recursively
    subdivide(0);
}

void BVH::updateNodeBounds(int nodeid) {
    BVHNode& node = nodes[nodeid];
    node.aabbMin = Vec3f(1e30f, 1e30f, 1e30f);
    node.aabbMax = Vec3f(-1e30f, -1e30f, -1e30f);
    for (int i = 0; i < node.triCount; i++)
    {
        uint first = node.leftFirst;
        Tri t = tris[first + i];
        node.aabbMin = min(node.aabbMin, t.v0);
        node.aabbMin = min(node.aabbMin, t.v1);
        node.aabbMin = min(node.aabbMin, t.v2);
        node.aabbMax = min(node.aabbMax, t.v0);
        node.aabbMax = min(node.aabbMax, t.v1);
        node.aabbMax = min(node.aabbMax, t.v2);
    }
}

void BVH::subdivide(int nodeid) {
    BVHNode node = nodes[nodeid];
    if (node.triCount <= 2) return;

    Vec3f extent = node.aabbMax - node.aabbMin;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;
    float splitPos = node.aabbMin[axis] + extent[axis] * 0.5f;

    int i = node.leftFirst;
    int j = i + node.triCount - 1;
    while (i <= j)
    {
        if (tris[i].centroid[axis] < splitPos)
            i++;
        else
            std::swap( tris[i], tris[j--] );
    }

    // abort split if one of the sides is empty
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount) return;
    // create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;
    nodes[leftChildIdx].leftFirst = node.leftFirst;
    nodes[leftChildIdx].triCount = leftCount;
    nodes[rightChildIdx].leftFirst = i;
    nodes[rightChildIdx].triCount = node.triCount - leftCount;
    node.leftFirst = leftChildIdx;
    node.triCount = 0;
    updateNodeBounds(leftChildIdx);
    updateNodeBounds(rightChildIdx);
    // recurse
    subdivide(leftChildIdx);
    subdivide(rightChildIdx);
}

void BVH::IntersectBVH( const Vec3f &orig, const Vec3f &dir, float &tnear, const uint nodeIdx, Vec3f &N)
{
    BVHNode& node = nodes[nodeIdx];
    if (!IntersectAABB( orig, dir, node.aabbMin, node.aabbMax, tnear)) return;
    if (node.triCount > 0)
    {
        for (int i = 0; i < node.triCount; i++ ){
            if (scene.ray_triangle_intersect(i, orig, dir, tnear)) {
                N = scene.triangle_normal(i).normalize();
            }
        }
    }
    else
    {
        IntersectBVH( orig, dir, tnear, node.leftFirst, N);
        IntersectBVH( orig, dir, tnear, node.leftFirst + 1, N);
    }
}

bool BVH::IntersectAABB(const Vec3f &orig, const Vec3f &dir, const Vec3f bmin, const Vec3f bmax, float &tnear)
{
    float tx1 = (bmin.x - orig.x) / dir.x, tx2 = (bmax.x - orig.x) / dir.x;
    float tmin = fmin( tx1, tx2 ), tmax = fmax( tx1, tx2 );
    float ty1 = (bmin.y - orig.y) / dir.y, ty2 = (bmax.y - orig.y) / dir.y;
    tmin = fmax( tmin, fmin( ty1, ty2 ) ), tmax = fmin( tmax, fmax( ty1, ty2 ) );
    float tz1 = (bmin.z - orig.z) / dir.z, tz2 = (bmax.z - orig.z) / dir.z;
    tmin = fmax( tmin, fmin( tz1, tz2 ) ), tmax = fmin( tmax, fmax( tz1, tz2 ) );
    return tmax >= tmin && tmin < tnear && tmax > 0;
}

bool BVH::scene_intersect(const Vec3f &orig, const Vec3f &dir,  Vec3f &hit, Vec3f &N){
    float t = std::numeric_limits<float>::max();
    IntersectBVH(orig, dir, t, 0, N);
    hit = orig + dir * t;
    return t < 1000;
}

float BVH::min(float f1, float f2){
    if (f1 < f2){
        return f1;
    } else {
        return f2;
    }
}
float BVH::max(float f1, float f2){
    if (f1 > f2){
        return f1;
    } else {
        return f2;
    }
}
Vec3f BVH::min(Vec3f f1, Vec3f f2){
    return Vec3f(min(f1.x, f2.x), min(f1.y, f2.y), min(f1.z, f2.z));
}
Vec3f BVH::max(Vec3f f1, Vec3f f2){
    return Vec3f(max(f1.x, f2.x), max(f1.y, f2.y), max(f1.z, f2.z));
}
