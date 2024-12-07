#include "common.h"
#include "accelerate.h"
#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "primitive.h"
#include "mesh.h"
#include "model.h"

#include <time.h>

int main(int argc, char* argv[])
{
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
    cam.stats->samples_per_pixel = 1;
    auto clkBuild = clock();
    std::clog <<"\rBuilding Done in "<< double(clkBuild - clkStart) / CLOCKS_PER_SEC << "s !                " << std::endl;

    // Run Renderer
    std::clog << "Starting Render to " << stng.outfile << std::endl;
    cam.render(world, stng.outfile.c_str());
    auto clkRender = clock();
    std::clog << "\rRendering Done in " << double(clkRender - clkBuild) / CLOCKS_PER_SEC << "s !                    " << std::endl;

    // Save traversal statistics
    cam.save_stats();

    // End clock counter
    auto clkFinish = clock();
    std::clog << "Total Clock Time: " << double(clkFinish - clkStart) / CLOCKS_PER_SEC << "s" << std::endl;

    return 0;
}
