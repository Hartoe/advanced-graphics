#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "material.h"
#include "model.h"
#include "primitive.h"

#include <cstring>

struct settings {
    std::string infile = "";
    std::string outfile = "";
    std::string model = "";
};

const settings parse_args(int argc, char* argv[]) {
    settings stng;
    for (int i = 1; i < argc; i++) {
        const char* opt = argv[i];
        
        // Check if flag, otherwise skip
        auto flag = std::string(opt).substr(0, 2);
        std::cout << flag << std::endl;
        std::cout << opt[0] << std::endl;
        if (opt[0] == '-'){
            if (++i < argc) {
                const char* param = argv[i+1];

                if (strcmp(flag.c_str(), "-m") == 0) {
                    stng.model = param;
                } else if (strcmp(flag.c_str(), "-i") == 0) {
                    stng.infile = param;
                } else if (strcmp(flag.c_str(), "-o") == 0) {
                    stng.outfile = param;
                }
            }
        }
    }

    return stng;
}

const hittable_list load_scene(const char* path, const char* mode) {
    hittable_list world;

    FILE* file = fopen(path, "r");
    if (file == NULL)
        return world;

    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "MODEL") == 0) {
            std::clog << "\rLoading Scene (Building Model)..." << std::flush;
            char model_path[128];
            char mat_str[3];
            double r, g, b;
            fscanf(file, "%s (%s", &model_path, &mat_str);
            shared_ptr<material> mat;
            if (strcmp(mat_str, "LAM") == 0) {
                fscanf(file, "%lf %lf %lf)", &r, &g, &b);
                mat = make_shared<lambertian>(color(r, g, b));
            }
            else if (strcmp(mat_str, "MET") == 0) {
                double fuzz;
                fscanf(file, "%lf %lf %lf %lf)", &r, &g, &b, &fuzz);
                mat = make_shared<metal>(color(r, g, b), fuzz);
            }
            else if (strcmp(mat_str, "DIE") == 0) {
                double index;
                fscanf(file, "%lf)", &index);
                mat = make_shared<dielectric>(index);
            }
            world.add(make_shared<model>(model_path, mat, mode));
        } else if (strcmp(lineHeader, "SPHERE") == 0) {
            std::clog << "\rLoading Scene (Building Sphere)..." << std::flush;
            char mat_str[3];
            double x, y, z;
            double radius;
            double r, g, b;
            fscanf(file, "%lf %lf %lf %lf (%s", &x, &y, &z, &radius, &mat_str);
            shared_ptr<material> mat;
            if (strcmp(mat_str, "LAM") == 0) {
                fscanf(file, "%lf %lf %lf)", &r, &g, &b);
                mat = make_shared<lambertian>(color(r, g, b));
            }
            else if (strcmp(mat_str, "MET") == 0) {
                double fuzz;
                fscanf(file, "%lf %lf %lf %lf)", &r, &g, &b, &fuzz);
                mat = make_shared<metal>(color(r, g, b), fuzz);
            }
            else if (strcmp(mat_str, "DIE") == 0) {
                double index;
                fscanf(file, "%lf)", &index);
                mat = make_shared<dielectric>(index);
            }
            world.add(make_shared<sphere>(point(x, y, z), radius, mat));
        }
    }
    fclose(file);

    return world;
}

#endif