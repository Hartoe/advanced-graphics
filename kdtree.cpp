#include "kdtree.h"

KDTree::KDTree(Model scene) : scene(scene)
{
    build_tree();
}

bool KDTree::scene_intersect(const Vec3f &orig, const Vec3f &dir, Vec3f &hit, Vec3f &N)
{
    float duck_dist = std::numeric_limits<float>::max();
    for (int i = 0; i < scene.nfaces(); i++) {
        float dist_i;
        if (scene.ray_triangle_intersect(i, orig, dir, dist_i) && dist_i < duck_dist) {
            duck_dist = dist_i;
            hit = orig + dir * dist_i;
            N = scene.triangle_normal(i).normalize();
        }
    }
    return duck_dist < 1000;
}

Node KDTree::build_tree()
{
    // Gets scene bounding box
    AABB bounds;
    scene.get_bbox(bounds.min, bounds.max);

    // Return root node
    return build_node(bounds, 0, scene.nfaces());
}

// Recursive call of node building
Node KDTree::build_node(AABB& bounds, int first, int count, int recursion)
{
    // Check if the termination metrics have been met
    if (terminate_tree(recursion, count))
    {
        // Create leaf node with leftover vertices
        LeafNode node;
        node.first = first;
        node.count = count;
        return node;
    }

    // Create a plane and split the bounding box
    Plane plane;
    AABB right, left;
    split_along_plane(bounds, left, right, plane);
    int leftFirst, leftCount, rightFirst, rightCount;

    // Sort faces by bounding boxes
    get_faces(plane, first, count, leftFirst, leftCount, rightFirst, rightCount);

    // Generate new interior node
    InteriorNode node;
    node.plane = plane;
    node.left = &build_node(left, leftFirst, leftCount, recursion + 1);
    node.right = &build_node(right, rightFirst, rightCount, recursion + 1);
    return node;
}

// Checks if there are less than 10 faces in a bounding box or recursion depth has been reached
bool terminate_tree(int recursion, int count)
{
    return count < 10 || recursion > 20;
}

// Finds an axis aligned plane within the bounding box
void split_along_plane(AABB& bounds, AABB& left, AABB& right, Plane& plane)
{
    // Get edge lengths
    Vec3f lengths = bounds.max - bounds.min;

    // Get longest edge
    int longestEdge = 0;
    if (lengths[1] > lengths[0] && lengths[1] > lengths[2])
        longestEdge = 1;
    else if (lengths[2] > lengths[0] && lengths[2] > lengths[1])
        longestEdge = 2;

    // Split longest edge and make the two new bounding boxes
    float offset = lengths[longestEdge] / 2.0f;
    AABB left, right;
    left.max = bounds.max;
    left.min = bounds.min;
    left.min[longestEdge] += offset;
    right.min = bounds.min;
    right.max = bounds.max;
    right.max[longestEdge] = left.min[longestEdge];

    // Generate the splitting plane of the two new bounding boxes
    Plane plane;
    plane.P0 = left.min;
    plane.P1 = right.max;
    Vec3f P3 = plane.P0;
    if (longestEdge == 0)
        P3[1] = plane.P1[1];
    else
        P3[0] = plane.P1[0];
    plane.N = cross(plane.P0 - plane.P1, plane.P0 - P3).normalize();
}

// Splits the faces based on a splitting plane
void get_faces(Plane& plane, int& first, int& count, int& leftFirst, int& leftCount, int& rightFirst, int& rightCount)
{
    // TODO: Quick sort the faces of the model around the plane as a pivot
}