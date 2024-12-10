# INFOMAGR - Ray-Tracer

Source code based on [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)

## How to run
1. Run the make file using the ```make``` command
  This assumes you are using the ```g++``` compiler, you can also manually compile using a different c++ compiler or change the make file.
2. Run the raytracer using the following command:
```
./main.exe -i ./path/to/input.trace -o ./path/to/output.ppm -m model-name
```
  ```model-name``` consists of either 'brute', 'bvh', 'kd', or 'bih'
                   Each abreviation stands for their own acceleration structure (except for brute, which is the absence of a structure).

**Warning!**
In case the program doesn't provide an output file or the accompanying traversal and intersection files, first create the directory ./output/ with a sub-directory ./output/stats/ 
