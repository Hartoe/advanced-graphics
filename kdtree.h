#include "model.h"
#include "geometry.h"
#include <vector>

struct Plane
{
    Vec3f P0, P1, N; // Start point, mid point, and plane normal
};

struct Node {};

struct InteriorNode : Node
{
    Plane plane;
    Node* left;
    Node* right;
};

struct LeafNode : Node
{
    int first; // index to first vertex
    int count; // amount of vertices in node
};

struct AABB
{
    Vec3f min; // minimum point of the cube
    Vec3f max; // maximum point of the cube
};

class KDTree {
private:
    Model scene;
public:
    KDTree(Model scene);
    bool scene_intersect(const Vec3f& orig, const Vec3f& dir, Vec3f& hit, Vec3f& N); // Material?
    Node build_tree();
    Node build_node(AABB& bounds, int first, int count, int recursion = 1);
};