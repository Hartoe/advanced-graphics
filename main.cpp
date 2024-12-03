#include "common.h"
#include "accelerate.h"
#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "primitive.h"
#include "mesh.h"
#include "model.h"

#include <time.h>

int main()
{
    // Get flags
    std::clog << "\rParsing Flags..." << std::flush;
    settings stng = parse_args(__argc, __argv);
    std::clog << "\rParsing Done!               \n" << std::flush;

    // Start clock counter
    auto clkStart = clock();

    // Read in .trace file
    std::clog << "\rLoading Scene..." << std::flush;
    hittable_list world = load_scene(
                                (strcmp(stng.infile.c_str(), "") != 0) ? stng.infile.c_str() : "scenes/in.trace",
                                (strcmp(stng.model.c_str(), "") != 0) ? stng.model.c_str()  : "bvh");

    std::clog <<"\rBuilding Done!                                \n" << std::flush;

    // Initialize Camera
    camera cam;

    cam.aspect_ratio = 1;
    cam.width = 400;
    cam.samples_per_pixel = 20;
    cam.stats->samples_per_pixel = 1;
    cam.max_depth = 20;

    cam.vfov = 90;
    cam.lookfrom = point(-0.04, 0.14, 0.11);
    cam.lookat = point(-0.04, 0.1, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    // Run Renderer
    std::clog << "\rStarting Render\n"  << std::flush;
    cam.render(world, (stng.outfile != "") ? stng.outfile.c_str() : "output/image.ppm");

    // End clock counter
    auto clkFinish = clock();
    std::clog << "Total Clock Time: " << clkFinish - clkStart << std::endl << std::flush;

    return 0;
}
