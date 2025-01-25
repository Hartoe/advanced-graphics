#include "common.h"
#include "camera.h"
#include "hittable.h"

#include <time.h>
#include <stdlib.h>
#ifdef __APPLE__
    #include <OpenCL/cl.h>
#elif defined _WIN32 || defined _WIN64
    #include <CL/cl.h>
#endif

int main(int argc, char* argv[])
{
    settings stng = parse_args(argc, argv);
    std::clog << "Building " << stng.infile << " with " << stng.model << " structure." << std::endl;

    // Start clock counter
    auto clkStart = clock();

    // Initialize Camera
    camera cam;

    // Read in .trace file
    std::clog << "Loading Scene..." << std::flush;
    hittable_list world = load_scene(cam, stng.infile.c_str(), stng.model.c_str());
    cam.initialize();
    auto clkBuild = clock();
    std::clog <<"\rBuilding Done in "<< double(clkBuild - clkStart) / CLOCKS_PER_SEC << "s !                " << std::endl;


    cl_int status = CL_SUCCESS;
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    size_t numNames = 0;

    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    platforms = (cl_platform_id*) malloc(numPlatforms*sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    status = clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, 0, NULL, &numNames);
    char Name[numNames];
    status = clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, sizeof(Name), Name, NULL);

    cl_uint numDevices;
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    cl_device_id *devices = NULL;
    devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    status = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &numNames);
    char deviceName[numNames];
    status = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);

    printf("Name of platform: %s\n", Name);
    printf("Name of device: %s\n", deviceName);

    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    if (status != CL_SUCCESS)
        std::cout << "CONTEXT: " << status << std::endl;

    cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, &status);
    if (status != CL_SUCCESS)
        std::cout << "QUEUE: " << status << std::endl;

    // int size = 10;
    // int A_h[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    // int B_h[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
    // int C_h[size];

    int n_pixels = cam.width * cam.height;
    int out_img_size = n_pixels * sizeof(uint);
    // int out_img_size = cam.width * cam.height * 4;
    std::cout << "Size: " << n_pixels << std::endl;
    cl_mem out_img = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_img_size, NULL, &status);

    // cl_mem A_d = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * size, NULL, &status);
    // cl_mem B_d = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * size, NULL, &status);
    // cl_mem C_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * size, NULL, &status);
    if (status != CL_SUCCESS)
        std::cout << "BUFFER: " << status << std::endl;

    // status = clEnqueueWriteBuffer(queue, A_d, CL_TRUE, 0, sizeof(int)*size, A_h, 0, NULL, NULL);
    // status = clEnqueueWriteBuffer(queue, B_d, CL_TRUE, 0, sizeof(int)*size, B_h, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        std::cout << "ENQUEUE: " << status << std::endl;

    // const char* kernelSource = R"(
    // __kernel void simple_add(__global const int *A,
    //                          __global const int *B,
    //                          __global int *C,
    //                          const unsigned int n) {
    //     int i = get_global_id(0);
    //     if (i < n) {
    //         C[i] = A[i] + B[i];
    //     }
    // })";
    const char* kernelSource = R"(
    __kernel void render( __global uint* out_img, const unsigned int width, const unsigned int height )
{
    // plot a pixel into the target array in GPU memory
    int threadIdx = get_global_id( 0 );
    int x = threadIdx % width;
    int y = threadIdx / width;
    int red = x / 3, green = y / 3;
    out_img[x + y * width] = (red << 16) + (green << 8);
})";

    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &status);
    if (status != CL_SUCCESS)
        std::cout << "PROGRAM: " << status << std::endl;

    status = clBuildProgram(program, 1, &(devices[0]), NULL, NULL, NULL);
    if (status != CL_SUCCESS){
        std::cout << "BUILD: " << status << std::endl;
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
    }
    status = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, NULL);
    // status = clGetProgramBuildInfo(program, &(devices[0]), CL_PROGRAM_BUILD_LOG, 0, NULL, NULL);
    if (status != CL_SUCCESS)
       std::cout << "BUILDINFO: " << status << std::endl;

    cl_kernel kernel = clCreateKernel(program, "render", &status);
    if (status != CL_SUCCESS)
       std::cout << "KERNEL: " << status << std::endl;
       
    // clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&A_d);
    // clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&B_d);
    // clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&C_d);
    // clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)&size);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&out_img);
    clSetKernelArg(kernel, 1, sizeof(unsigned int), (void*)&cam.width);
    clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)&cam.height);

    size_t global_size = n_pixels;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    uint out_img_cpu[n_pixels];
    clEnqueueReadBuffer(queue, out_img, CL_TRUE, 0, out_img_size, out_img_cpu, 0, NULL, NULL);

    clReleaseMemObject(out_img);
    // clReleaseMemObject(A_d);
    // clReleaseMemObject(B_d);
    // clReleaseMemObject(C_d);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    // for (int i = 0; i < size; i++) {
    //     std::cout << "C[" << i << "] = " << C_h[i] << std::endl;
    // }


    std::ofstream image(stng.outfile);

    image << "P3\n" << cam.width << ' ' << cam.height << "\n255\n";
    // Draw write colors to image
    for (int i = 0; i < n_pixels; i++){
        int c = out_img_cpu[i];
        int r, g, b;
        r = (c >> 16) & 255;
        g = (c >> 8) & 255;
        b = c & 255;
        image << r << ' ' << g << ' ' << b << '\n';
        image.flush();
    }
    image.close();

    auto clkFinish = clock();
    std::clog << "Total Clock Time: " << double(clkFinish - clkStart) / CLOCKS_PER_SEC << "s" << std::endl;
    return 0;

    // // Get flags
    // settings stng = parse_args(argc, argv);
    // std::clog << "Building " << stng.infile << " with " << stng.model << " structure." << std::endl;
    //
    // // Start clock counter
    // auto clkStart = clock();
    //
    // // Initialize Camera
    // camera cam;
    //
    // // Read in .trace file
    // std::clog << "Loading Scene..." << std::flush;
    // hittable_list world = load_scene(cam, stng.infile.c_str(), stng.model.c_str());
    // auto clkBuild = clock();
    // std::clog <<"\rBuilding Done in "<< double(clkBuild - clkStart) / CLOCKS_PER_SEC << "s !                " << std::endl;
    //
    // // Run Renderer
    // std::clog << "Starting Render to " << stng.outfile << std::endl;
    // cam.render(world, stng.outfile.c_str());
    // auto clkRender = clock();
    // std::clog << "\rRendering Done in " << double(clkRender - clkBuild) / CLOCKS_PER_SEC << "s !                        " << std::endl;
    //
    // // Save traversal statistics
    // std::clog << "Starting Stat Collection." << std::endl;
    // cam.save_stats(stng.outfile);
    // auto clkFinish = clock();
    // std::clog << "\rStat Collection Done in " << double(clkFinish - clkRender) / CLOCKS_PER_SEC << "s !                         " << std::endl;
    //
    // // End clock counter    
    // std::clog << "Total Clock Time: " << double(clkFinish - clkStart) / CLOCKS_PER_SEC << "s" << std::endl;
    //
    // return 0;
}
