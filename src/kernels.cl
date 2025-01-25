struct Intersection
{
    float t;                    // intersection distance along ray
    float u, v;	                // barycentric coordinates of the intersection
    uint instPrim;              // instance index (12 bit) and primitive index (20 bit)
};
 
struct Ray
{
    float3 O, D, rD;            // in OpenCL, each of these will be padded to 16 bytes
    struct Intersection hit;    // total ray size: 64 bytes
};

inline uint RGB32FtoRGB8( float3 c )
{
    int r = (int)(min( c.x, 1.f ) * 255);
    int g = (int)(min( c.y, 1.f ) * 255);
    int b = (int)(min( c.z, 1.f ) * 255);
    return (r << 16) + (g << 8) + b;
}

float3 Trace( struct Ray* ray )
{
    return (float3)( 1, 1, 1 );
}

__kernel void render( __global uint* target, uint width, uint height, float3 camPos, float3 p0, float3 dx, float3 dy )
{
    // plot a pixel into the target array in GPU memory
    int threadIdx = get_global_id( 0 );
    int x = threadIdx % width;
    int y = threadIdx / width;
    // create a primary ray for the pixel
    struct Ray ray;
    ray.O = camPos;
    float3 pixelPos = p0 + x * dx + y * dy;
    ray.D = normalize( pixelPos - ray.O );
    ray.hit.t = 1e30f; // 1e30f denotes 'no hit'
    // trace the primary ray
    float3 color = Trace( &ray );
    // plot the result
    target[x + y * width] = RGB32FtoRGB8( color );
}
