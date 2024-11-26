#include "model.h"
#include "geometry.h"
#include <vector>
class BVH {
private:
    Model scene;
public:
    BVH(Model scene);
    bool scene_intersect(const Vec3f &orig, const Vec3f &dir,  Vec3f &hit, Vec3f &N); // Material?
};
