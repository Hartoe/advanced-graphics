#include <opencl-c-base.h>
struct Intersection
{
    float t;                    // intersection distance along ray
    float u, v;	                // barycentric coordinates of the intersection
};
 
struct Ray
{
    float3 O, D, rD;            // in OpenCL, each of these will be padded to 16 bytes
    struct Intersection hit;    // total ray size: 64 bytes
};

struct Tri 
{ 
    float v0x, v0y, v0z;
    float v1x, v1y, v1z;
    float v2x, v2y, v2z;
    float cx, cy, cz;
};

bool IntersectTri( struct Ray* ray, __global struct Tri* tri)
{
    float3 v0 = (float3)(tri->v0x, tri->v0y, tri->v0z);
    float3 v1 = (float3)(tri->v1x, tri->v1y, tri->v1z);
    float3 v2 = (float3)(tri->v2x, tri->v2y, tri->v2z);
    float3 edge1 = v1 - v0, edge2 = v2 - v0;
    float3 h = cross( ray->D, edge2 );
    float a = dot( edge1, h );
    if (a > -0.00001f && a < 0.00001f) return false; // ray parallel to triangle
    float f = 1 / a;
    float3 s = ray->O - v0;
    float u = f * dot( s, h );
    if (u < 0 || u > 1) return false;
    float3 q = cross( s, edge1 );
    float v = f * dot( ray->D, q );
    if (v < 0 || u + v > 1) return false;
    float t = f * dot( edge2, q );
    if (t > 0.0001f && t < ray->hit.t)
        ray->hit.t = t, ray->hit.u = u,
        ray->hit.v = v;
    return true;
}

struct BVHNode
{
    float minx, miny, minz;
    int leftFirst;
    float maxx, maxy, maxz;
    int triCount;
};

inline uint RGB32FtoRGB8( float3 c )
{
    int r = (int)(min( c.x, 1.f ) * 255);
    int g = (int)(min( c.y, 1.f ) * 255);
    int b = (int)(min( c.z, 1.f ) * 255);
    return (r << 16) + (g << 8) + b;
}

float3 Trace( struct Ray* ray, __global struct Tri* tri)
{
    // return ray->D;
    for (uint i = 0; i < 508; i++)
    {
        IntersectTri(ray, &tri[i]);
    }
    if (ray->hit.t < 9999)
        return (float3)( 1, 1, 1 );
    else
        return (float3)( 0, 0, 0 );
}

bool intersect(struct BVHNode* node, struct Ray* ray)
{
    float adinv = 1.0 / ray->D.x;
    float t0 = (node->minx - ray->O.x) * adinv;
    float t1 = (node->maxx - ray->O.x) * adinv;
    if (t0 < t1)
    {
        if (t0 > ray->hit.u) ray->hit.u = t0;
        if (t1 < ray->hit.v) ray->hit.v = t1;
    }
    else
    {
        if (t1 > ray->hit.u) ray->hit.u = t1;
        if (t0 < ray->hit.v) ray->hit.v = t0;
    }
    if (ray->hit.v <= ray->hit.u)
        return false;

    adinv = 1.0 / ray->D.y;
    t0 = (node->miny - ray->O.y) * adinv;
    t1 = (node->maxy - ray->O.y) * adinv;
    if (t0 < t1)
    {
        if (t0 > ray->hit.u) ray->hit.u = t0;
        if (t1 < ray->hit.v) ray->hit.v = t1;
    }
    else
    {
        if (t1 > ray->hit.u) ray->hit.u = t1;
        if (t0 < ray->hit.v) ray->hit.v = t0;
    }
    if (ray->hit.v <= ray->hit.u)
        return false;

    adinv = 1.0 / ray->D.z;
    t0 = (node->minz - ray->O.z) * adinv;
    t1 = (node->maxz - ray->O.z) * adinv;
    if (t0 < t1)
    {
        if (t0 > ray->hit.u) ray->hit.u = t0;
        if (t1 < ray->hit.v) ray->hit.v = t1;
    }
    else
    {
        if (t1 > ray->hit.u) ray->hit.u = t1;
        if (t0 < ray->hit.v) ray->hit.v = t0;
    }
    if (ray->hit.v <= ray->hit.u)
        return false;

    return true;
}

__kernel void traverse( __global struct BVHNode* BVH, __global struct Tri* triData, __global const struct Ray *rays, __global float3* output )
{
    // Get the ray used by the instance
    struct Ray ray = rays[get_global_id(0)];

    int stack[64];
    int* stackPtr = stack;
    int index = 0;
    
    do
    {
        struct BVHNode node = BVH[index];
        // It is a leaf
        if (node.triCount > 0)
        {
            bool hit;
            for (int i = 0; i < node.triCount; i++)
            {
                hit = IntersectTri(&ray, &triData[node.leftFirst+i]);
            }
            output[get_global_id(0)] = (hit) ? (float3)(1, 1, 1) : (float3)(0, 0, 0);
        }
        // It is a child
        else
        {
            struct BVHNode childL = BVH[node.leftFirst];
            struct BVHNode childR = BVH[node.leftFirst+1];

            bool overlapL = intersect(&childL, &ray);
            bool overlapR = intersect(&childR, &ray);

            if (!overlapL && !overlapR)
                index = *--stackPtr;
            else
            {
                node = (overlapL) ? childL : childR;
                if (overlapL && overlapR)
                    *stackPtr++ = node.leftFirst+1;
            }
        }
    }
    while (index != NULL);
}

// Based on Jacco Bikkers gpu implementation of a bvh
__kernel void render( __global struct Tri* triData, __global uint* target, uint width, uint height, float3 camPos, float3 p0, float3 du, float3 dv)
{
    // plot a pixel into the target array in GPU memory
    int threadIdx = get_global_id( 0 );
    int x = threadIdx % width;
    int y = threadIdx / width;
    // create a primary ray for the pixel
    struct Ray ray;
    ray.O = camPos;
    float3 pixelPos = p0 + x * du + y * dv;
    ray.D = normalize( pixelPos - ray.O );
    ray.hit.t = 1e30f; // 1e30f denotes 'no hit'
    // trace the primary ray
    float3 color = Trace( &ray, triData);
    // plot the result
    target[x + y * width] = RGB32FtoRGB8(color);
}

uint3 to_morton_part(int val, int offset){
    uint3 res;
    for (uint i = 0; i < sizeof(uint); i++){
        uint v = 1 & (val >> 0);
        uint t = 0 * 3 + offset;
        uint lt = t % sizeof(uint);
        uint v2 = v << lt;
        int ax = t / 3;
        if (ax == 0) {
            res.x = res.x | v2;
        } else if (ax == 1) {
            res.y = res.y | v2;
        } else {
            res.z = res.z | v2;
        }
    }
    return res;
}

__kernel void to_morton_codes(__global struct Tri* triData, __global uint3* target, uint count)
{
    int threadIdx = get_global_id( 0 );
    int tri_idx = threadIdx % count;
    struct Tri t = triData[tri_idx];
    int3 val = convert_int3( (float3) (t.cx, t.cy, t.cz));
    uint3 px = to_morton_part(val.x, 0);
    uint3 py = to_morton_part(val.y, 1);
    uint3 pz = to_morton_part(val.z, 2);
    uint3 combined;
    combined.x = px.x | py.x | pz.x;
    combined.y = px.y | py.y | pz.y;
    combined.z = px.x | py.z | pz.z;
    target[tri_idx] = combined;
}
