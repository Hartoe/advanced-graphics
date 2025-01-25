__kernel void render( __global uint* out_img, const unsigned int width, const unsigned int height )
{
    // plot a pixel into the target array in GPU memory
    int threadIdx = get_global_id( 0 );
    int x = threadIdx % width;
    int y = threadIdx / width;
    int red = x / 3, green = y / 3;
    out_img[x + y * width] = (red << 16) + (green << 8);
}
