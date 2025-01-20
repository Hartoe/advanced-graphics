#include "common.h"
#include "accelerate.h"
#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "primitive.h"
#include "mesh.h"
#include "model.h"

#include <time.h>
#include <stdlib.h>
#include <CL/cl.h>

int main(int argc, char* argv[])
{
    cl_int status;
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

    cl_int error = 0;
    cl_context context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &error);
    if (error != 0)
        printf("ERROR! %i\n", error);

    printf("Name of platform: %s\n", Name);
    printf("Name of device: %s\n", deviceName);

    return 0;

    // Get flags
    settings stng = parse_args(argc, argv);
    std::clog << "Building " << stng.infile << " with " << stng.model << " structure." << std::endl;

    // Start clock counter
    auto clkStart = clock();

    // Initialize Camera
    camera cam;

    // Read in .trace file
    std::clog << "Loading Scene..." << std::flush;
    hittable_list world = load_scene(cam, stng.infile.c_str(), stng.model.c_str());
    auto clkBuild = clock();
    std::clog <<"\rBuilding Done in "<< double(clkBuild - clkStart) / CLOCKS_PER_SEC << "s !                " << std::endl;

    // Run Renderer
    std::clog << "Starting Render to " << stng.outfile << std::endl;
    cam.render(world, stng.outfile.c_str());
    auto clkRender = clock();
    std::clog << "\rRendering Done in " << double(clkRender - clkBuild) / CLOCKS_PER_SEC << "s !                        " << std::endl;

    // Save traversal statistics
    std::clog << "Starting Stat Collection." << std::endl;
    cam.save_stats(stng.outfile);
    auto clkFinish = clock();
    std::clog << "\rStat Collection Done in " << double(clkFinish - clkRender) / CLOCKS_PER_SEC << "s !                         " << std::endl;

    // End clock counter    
    std::clog << "Total Clock Time: " << double(clkFinish - clkStart) / CLOCKS_PER_SEC << "s" << std::endl;

    return 0;
}
