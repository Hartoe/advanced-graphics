#ifndef PARSER_H
#define PARSER_H

#include "camera.h"
#include "common.h"
#include "material.h"
#include "model.h"
#include "primitive.h"

#include <cstring>

struct settings {
    std::string infile = "scenes/in.trace";
    std::string outfile = "output/image.ppm";
    std::string model = "bvh";
};

const settings parse_args(int argc, char* argv[]) {
    settings stng;
    for (int i = 1; i < argc; i++) {
        const char* opt = argv[i];

        // Check if flag, otherwise skip
        auto flag = std::string(opt).substr(0, 2);
        if (opt[0] == '-') {
            if ((i + 1) < argc) {
                const char* param = argv[i + 1];

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

    std::cout << "Loading Scene (Image Info)...               " << std::endl;

    return stng;
}

const void parse_image_info(FILE* file, camera& cam) {
    std::cout << "Loading Scene (Image Info)...               " << std::endl;
    int width;
    double aspect_w, aspect_h;
    fscanf(file, "%i %lf/%lf", &width, &aspect_w, &aspect_h);
    cam.width = width;
    cam.aspect_ratio = aspect_w / aspect_h;
    std::cout << "Image: " << width << " width, " << aspect_w << '/' << aspect_h << "             " << std::endl;
}

const void parse_camera_info(FILE* file, camera& cam) {
    std::cout << "Loading Scene (Camera Info)...              " << std::endl;
    double lfx, lfy, lfz;
    double lax, lay, laz;
    int upx, upy, upz;
    double fov;
    fscanf(file, " (%lf %lf %lf) (%lf %lf %lf) (%i %i %i) %lf", &lfx, &lfy, &lfz, &lax, &lay, &laz, &upx, &upy, &upz, &fov);
    cam.lookfrom = point(lfx, lfy, lfz);
    cam.lookat = point(lax, lay, laz);
    cam.vup = vec3(upx, upy, upz);
    cam.vfov = fov;
    std::cout << "Camera: (" << lfx << ' ' << lfy << ' ' << lfz << ") (" << lax << ' ' << lay << ' ' << laz << ") (" << upx << ' ' << upy << ' ' << upz << ") " << fov << std::endl;
}

const void parse_aa(FILE* file, camera& cam) {
    std::cout << "Loading Scene (Anti-Aliasing)...            " << std::endl;
    int samples;
    fscanf(file, "%i", &samples);
    cam.samples_per_pixel = samples;
    std::cout << "Anti-Aliasing: " << samples << " samples per pixel         " << std::endl;
}

const void parse_depth(FILE* file, camera& cam) {
    std::cout << "Loading Scene (Shadow Depth)...             " << std::endl;
    int depth;
    fscanf(file, "%i", &depth);
    cam.max_depth = depth;
    std::cout << "Max Bounce Depth: " << depth << "               " << std::endl;
}

const void parse_blur(FILE* file, camera& cam) {
    std::cout << "Loading Scene (Defocus Blur)...             " << std::endl;
    double blur;
    fscanf(file, "%lf", &blur);
    cam.defocus_angle = blur;
    std::cout << "Defocus Angle: " << blur << "                   " << std::endl;
}

const shared_ptr<material> parse_material(FILE* file) {
    std::cout << "\rParsing Material...             " << std::endl;
    char c1, c2, c3;
    fscanf(file, "(%c%c%c", &c1, &c2, &c3);

    double r, g, b;
    if (c1 == 'L') {
        fscanf(file, "%lf %lf %lf)", &r, &g, &b);
        return make_shared<lambertian>(color(r, g, b));
    } else if (c1 == 'M') {
        double fuzz;
        fscanf(file, "%lf %lf %lf %lf)", &r, &g, &b, &fuzz);
        return make_shared<metal>(color(r, g, b), fuzz);
    } else if (c1 == 'D') {
        double index;
        fscanf(file, "%lf)", &index);
        return make_shared<dielectric>(index);
    }

    throw std::invalid_argument("Could not parse Material!");
}

const shared_ptr<model> parse_model(FILE* file, const char* mode) {
    std::cout << "Loading Scene (Building Model)...           " << std::endl;
    char model_path[128];
    std::cout << file << std::flush;
    fscanf(file, "%s ", model_path);
    shared_ptr<material> mat = parse_material(file);
    return make_shared<model>(model_path, mat, mode);
}

const shared_ptr<sphere> parse_sphere(FILE* file) {
    std::cout << "Loading Scene (Building Sphere)...          " << std::endl;
    double x, y, z;
    double radius;
    fscanf(file, " (%lf %lf %lf) %lf ", &x, &y, &z, &radius);
    shared_ptr<material> mat = parse_material(file);
    return make_shared<sphere>(point(x, y, z), radius, mat);
}

const shared_ptr<quad> parse_quad(FILE* file) {
    std::cout << "Loading Scene (Building Quad)...            " << std::endl;
    double qx, qy, qz;
    double ux, uy, uz;
    double vx, vy, vz;
    fscanf(file, " (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) ", &qx, &qy, &qz, &ux, &uy, &uz, &vx, &vy, &vz);
    shared_ptr<material> mat = parse_material(file);
    return make_shared<quad>(point(qx, qy, qz), vec3(ux, uy, uz), vec3(vx, vy, vz), mat);
}

const hittable_list load_scene(camera& cam, const char* path, const char* mode) {
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
            world.add(parse_model(file, mode));
        } else if (strcmp(lineHeader, "SPHERE") == 0) {
            world.add(parse_sphere(file));
        } else if (strcmp(lineHeader, "QUAD") == 0) {
            world.add(parse_quad(file));
        } else if (strcmp(lineHeader, "IMAGE") == 0) {
            parse_image_info(file, cam);
        } else if (strcmp(lineHeader, "CAM") == 0) {
            parse_camera_info(file, cam);
        } else if (strcmp(lineHeader, "AA") == 0) {
            parse_aa(file, cam);
        } else if (strcmp(lineHeader, "DEPTH") == 0) {
            parse_depth(file, cam);
        } else if (strcmp(lineHeader, "BLUR") == 0) {
            parse_blur(file, cam);
        }
    }
    fclose(file);

    return world;
}

#endif
