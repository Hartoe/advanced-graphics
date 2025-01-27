# INFOMAGR - Ray-Tracer

Source code based on [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)

## Assignment 2

For the second assignment our goals were to move the traversal and the construction of the BVH acceleration structure to the GPU with the use of OpenCL. To achieve this, we had to adapt and rework our older code to make this work.

The approach we first took was to make sure OpenCL could run on both our machines and run a simple kernel program. After this was successful, we started moving our BVH construction and traversal code into OpenCL code in the `kernels.cl` file. To supplement this method, we reworked code from our `parser.h` to read in the kernel code.

In general, our approach for construction of the BVH would be as follows: Use a Z-order curve defined by morton codes to represent the 3d points of the BVH, which can then be used to representour BVH as an array.

After our construction, we use a kernel program that gets given a BVH and a list of rays, where each instance on the GPU calculates the intersection of one of the rays with the BVH. This is done by adapting the BVH Node structure as represented in the lecture slides, together with information found on NVIDIA's web page on parallel traversal.

All in all, the added speed of the construction and traversal on the GPU would speed up the ray-tracer by a significant amount.

Sadly, due to illness, a switch in framework, and multiple issues with working congruently on Mac and Windows, we could not finish the complete implementation in time for the assignments deadline. We have implemented most of the construction and traversal code in the `kernels.cl` file, but have yet to fully incorporate it within the `main.cpp` file. As such, building and running the executable does not show off the full power of our implementation yet.


## Assignment 1

## How to run
1. Run the make file using the ```make``` command<br/>
  This assumes you are using the ```g++``` compiler, you can also manually compile using a different c++ compiler or change the make file.
2. Run the raytracer using the following command:
```
./main.exe -i ./path/to/input.trace -o ./path/to/output.ppm -m model-name
```
  ```model-name``` consists of either 'brute', 'bvh', 'kd', or 'bih'
                   Each abreviation stands for their own acceleration structure (except for brute, which is the absence of a structure).

**Warning!**<br/>
In case the program doesn't provide an output file or the accompanying traversal and intersection files, first create the directory ./output/ with a sub-directory ./output/stats/ 