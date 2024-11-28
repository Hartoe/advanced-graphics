#include "model.h"
#include "geometry.h"
#include <vector>
class BVH {
private:
    struct Tri{
        Vec3f v0;
        Vec3f v1;
        Vec3f v2;
        Vec3f centroid;
    };
    struct BVHNode
    {
        Vec3f aabbMin, aabbMax;
        int leftFirst, triCount;
    };
    Model scene;
    std::vector<Tri> tris;
    std::vector<BVHNode> nodes;
    void updateNodeBounds(int nodeid);
    void subdivide(int nodeid);
    Vec3f min(Vec3f f1, Vec3f f2);
    Vec3f max(Vec3f f1, Vec3f f2);
    float min(float f1, float f2);
    float max(float f1, float f2);
    void IntersectBVH( const Vec3f &orig, const Vec3f &dir, float &tnear, const uint nodeIdx, Vec3f &N );
    bool IntersectAABB(const Vec3f &orig, const Vec3f &dir, const Vec3f bmin, const Vec3f bmax, float &tnear);
    int nodesUsed = 1;
public:
    BVH(Model scene);
    int triangle_intersection_tests = 0;
    int traversal_steps = 0;
    bool scene_intersect(const Vec3f &orig, const Vec3f &dir,  Vec3f &hit, Vec3f &N); // Material?
};
