#include "common.h"
#include "camera.h"
#include "hittable.h"

#include <time.h>
#include <stdlib.h>
#include <vector>
#ifdef __APPLE__
    #include <OpenCL/cl.h>
#elif defined _WIN32 || defined _WIN64
    #include <CL/cl.h>
#endif

static std::string readStringFromFile(
    const std::string& filename )
{
    std::ifstream is(filename, std::ios::binary);
    if (!is.good()) {
        printf("Couldn't open file '%s'!\n", filename.c_str());
        return "";
    }

    size_t filesize = 0;
    is.seekg(0, std::ios::end);
    filesize = (size_t)is.tellg();
    is.seekg(0, std::ios::beg);

    std::string source{
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>() };

    return source;
}

struct Tri 
{ 
    float v0x, v0y, v0z;
    float v1x, v1y, v1z;
    float v2x, v2y, v2z;
    float cx, cy, cz;
};

struct F3
{
    float x, y, z;
    float dummy;
};

F3 tof3(vec3 v){
    F3 res;
    res.x = v.x();
    res.y = v.y();
    res.z = v.z();
    return res;
}

bool loadOBJ(const char* path, Tri *tris) {
    FILE* file = fopen(path, "r");
    if (file == NULL)
        return false;

    std::vector<vec3> vertices;
    int i =0;
    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0) {
            double x, y, z;
            fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
            vertices.push_back(vec3(x, y, z));
        } else if (strcmp(lineHeader, "f") == 0) {
            unsigned int v1, v2, v3;
            fscanf(file, "%d %d %d\n", &v1, &v2, &v3);
            Tri t;
            t.v0x = vertices[v1].x();
            t.v0y = vertices[v1].y();
            t.v0z = vertices[v1].z();
            t.v1x = vertices[v2].x();
            t.v1y = vertices[v2].y();
            t.v1z = vertices[v2].z();
            t.v2x = vertices[v3].x();
            t.v2y = vertices[v3].y();
            t.v2z = vertices[v3].z();
            t.cx = (t.v0x + t.v1x + t.v2x) / 3;
            t.cy = (t.v0y + t.v1y + t.v2y) / 3;
            t.cz = (t.v0z + t.v1z + t.v2z) / 3;
            tris[i] = t;
            i++;
        }
    }
    std::cout << "tri size:" << sizeof(Tri) << std::endl;
    std::cout << "tri count:" << i << std::endl;
    fclose(file);

    return true;
}

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
    Tri tris[508];
    loadOBJ("models/duck.obj", tris);
    int n_tris = 508;
    std::cout <<"n_tris:"<< n_tris << std::endl;
    
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

    int n_pixels = cam.width * cam.height;
    int out_img_size = n_pixels * sizeof(unsigned int);
    // int out_img_size = cam.width * cam.height * 4;
    std::cout << "Size: " << n_pixels << std::endl;
    cl_mem out_img = clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_img_size, NULL, &status);
    cl_mem tri_buff = clCreateBuffer(context, CL_MEM_READ_ONLY, n_tris * sizeof(Tri), NULL, &status);
    if (status != CL_SUCCESS)
        std::cout << "BUFFER: " << status << std::endl;
    status = clEnqueueWriteBuffer(queue, tri_buff, CL_TRUE, 0, sizeof(Tri)*n_tris, tris, 0, NULL, NULL);
    Tri tris2[2];
    tris2[0].cx = 1;
    tris2[0].cy = 1;
    tris2[0].cz = 1;
    tris2[1].cx = 2;
    tris2[1].cy = 2;
    tris2[1].cz = 2;
    uint codes[8];
    cl_mem out_codes = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 2 * 4 * sizeof(uint), NULL, &status);
    cl_mem tri_buff2 = clCreateBuffer(context, CL_MEM_READ_ONLY, 2 * sizeof(Tri), NULL, &status);
    if (status != CL_SUCCESS)
        std::cout << "BUFFER: " << status << std::endl;
    status = clEnqueueWriteBuffer(queue, tri_buff2, CL_TRUE, 0, sizeof(Tri)*2, tris2, 0, NULL, NULL);
    if (status != CL_SUCCESS)
        std::cout << "ENQUEUE: " << status << std::endl;

    std::string s =readStringFromFile("src/kernels.cl");
    const char* kernelSource = s.c_str();


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
    if (status != CL_SUCCESS)
       std::cout << "BUILDINFO: " << status << std::endl;

    cl_kernel kernel = clCreateKernel(program, "render", &status);
    if (status != CL_SUCCESS)
       std::cout << "KERNEL: " << status << std::endl;

    cl_kernel kernel2 = clCreateKernel(program, "to_morton_codes", &status);
    if (status != CL_SUCCESS)
       std::cout << "KERNEL: " << status << std::endl;
    clSetKernelArg(kernel2, 0, sizeof(cl_mem), (void*)&tri_buff2);
    clSetKernelArg(kernel2, 1, sizeof(cl_mem), (void*)&out_codes);
    uint count2 = 2;
    clSetKernelArg(kernel2, 2, sizeof(unsigned int), (void*)&count2);
       
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&tri_buff);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&out_img);
    clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)&cam.width);
    clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)&cam.height);
    F3 camO = tof3(cam.lookfrom);
    F3 camtl = tof3(cam.pixel00_loc);
    F3 camdu = tof3(cam.pixel_delta_u);
    F3 camdv = tof3(cam.pixel_delta_v);
    clSetKernelArg(kernel, 4, sizeof(F3), (void*)&camO);
    clSetKernelArg(kernel, 5, sizeof(F3), (void*)&camtl);
    clSetKernelArg(kernel, 6, sizeof(F3), (void*)&camdu);
    clSetKernelArg(kernel, 7, sizeof(F3), (void*)&camdv);
    std::cout << "dx.x" << cam.pixel_delta_u.x() << std::endl;
    std::cout << "dx.y" << cam.pixel_delta_u.y() << std::endl;
    std::cout << "dx.z" << cam.pixel_delta_u.z() << std::endl;
    std::cout << "dy" << cam.pixel_delta_v << std::endl;
    std::cout << "from" << cam.lookfrom << std::endl;
    std::cout << "00" << cam.pixel00_loc << std::endl;

    size_t global_size = n_pixels;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    unsigned int out_img_cpu[n_pixels];
    size_t global_size2 = 2;
    clEnqueueNDRangeKernel(queue, kernel2, 1, NULL, &global_size2, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, out_img, CL_TRUE, 0, out_img_size, out_img_cpu, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, out_codes, CL_TRUE, 0, 8*sizeof(uint), codes, 0, NULL, NULL);

    clReleaseMemObject(out_img);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    for (int i = 0; i < 8; i++){
        std::cout << "codes: " << codes[i] << std::endl;
    }
    std::ofstream image(stng.outfile);

    image << "P3\n" << cam.width << ' ' << cam.height << "\n255\n";
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
