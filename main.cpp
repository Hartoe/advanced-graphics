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

int main(int argc, char* argv[])
{
    // Get flags
    settings stng = parse_args(argc, argv);
    std::clog << "\rBuilding " << stng.infile << " with " << stng.model << " structure.\n" << std::flush;

    // Start clock counter
    auto clkStart = clock();

    // Initialize Camera
    camera cam;

    // Read in .trace file
    std::clog << "\rLoading Scene..." << std::flush;
    hittable_list world = load_scene(cam, stng.infile.c_str(), stng.model.c_str());
    cam.stats->samples_per_pixel = 1;
    auto clkBuild = clock();
    std::clog <<"\rBuilding Done in "<< double(clkBuild - clkStart) / CLOCKS_PER_SEC << "s !                \n" << std::flush;

    // Run Renderer
    std::clog << "\rStarting Render to " << stng.outfile << "\n"  << std::flush;
    cam.render(world, stng.outfile.c_str());
    auto clkRender = clock();
    std::clog << "\rRendering Done in " << double(clkRender - clkBuild) / CLOCKS_PER_SEC << "s !                    \n" << std::flush;

    // Save traversal statistics
    cam.save_stats();

    // End clock counter
    auto clkFinish = clock();
    std::clog << "Total Clock Time: " << double(clkFinish - clkStart) / CLOCKS_PER_SEC << "s" << std::endl << std::flush;

    return 0;
}
