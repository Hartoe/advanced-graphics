#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "camera.h"
#include "hittable.h"
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

                if (strcmp(opt, "-m") == 0 || strcmp(opt, "--model") == 0) {
                    stng.model = param;
                } else if (strcmp(opt, "-i") == 0 || strcmp(opt, "--input") == 0) {
                    stng.infile = param;
                } else if (strcmp(opt, "-o") == 0 || strcmp(opt, "--output") == 0) {
                    stng.outfile = param;
                }
            }
        }
    }

    return stng;
}

const void parse_image_info(FILE* file, camera& cam) {
    std::clog << "\rLoading Scene (Image Info)...               " << std::flush;
    int width;
    double aspect_w, aspect_h;
    fscanf(file, "%i %lf/%lf", &width, &aspect_w, &aspect_h);
    cam.width = width;
    cam.aspect_ratio = aspect_w / aspect_h;
    std::clog << "\rImage: " << width << " width, " << aspect_w << '/' << aspect_h << "             " << std::endl;
}

const void parse_camera_info(FILE* file, camera& cam) {
    std::clog << "\rLoading Scene (Camera Info)...              " << std::flush;
    double lfx, lfy, lfz;
    double lax, lay, laz;
    int upx, upy, upz;
    double fov;
    fscanf(file,
            " (%lf %lf %lf) (%lf %lf %lf) (%i %i %i) %lf",
            &lfx, &lfy, &lfz, &lax, &lay, &laz, &upx, &upy, &upz, &fov);
    cam.lookfrom = point(lfx, lfy, lfz);
    cam.lookat = point(lax, lay, laz);
    cam.vup = vec3(upx, upy, upz);
    cam.vfov = fov;
    std::clog << "\rCamera: (" << lfx << ' ' << lfy << ' ' << lfz << ") ("
        << lax << ' ' << lay << ' ' << laz << ") (" << upx << ' ' << upy
        << ' ' << upz << ") " << fov << std::endl;
}

const void parse_aa(FILE* file, camera& cam) {
    std::clog << "\rLoading Scene (Anti-Aliasing)...            " << std::flush;
    int samples;
    fscanf(file, "%i", &samples);
    cam.samples_per_pixel = samples;
    std::clog << "\rAnti-Aliasing: " << samples << " samples per pixel         " << std::endl;
}

const void parse_depth(FILE* file, camera& cam) {
    std::clog << "\rLoading Scene (Shadow Depth)...             " << std::flush;
    int depth;
    fscanf(file, "%i", &depth);
    cam.max_depth = depth;
    std::clog << "\rMax Bounce Depth: " << depth << "               " << std::endl;
}

const void parse_blur(FILE* file, camera& cam) {
    std::clog << "\rLoading Scene (Defocus Blur)...             " << std::flush;
    double blur;
    fscanf(file, "%lf", &blur);
    cam.defocus_angle = blur;
    std::clog << "\rDefocus Angle: " << blur << "                   " << std::endl;
}

const shared_ptr<material> parse_material(FILE* file) {
    char c1, c2, c3;
    fscanf(file, "(%c%c%c", &c1, &c2, &c3);
    double r, g, b;
    if (c1 == 'L') {
        fscanf(file, "%lf %lf %lf)", &r, &g, &b);
        return make_shared<lambertian>(color(r, g, b));
    }
    else if (c1 == 'M') {
        double fuzz;
        fscanf(file, "%lf %lf %lf %lf)", &r, &g, &b, &fuzz);
        return make_shared<metal>(color(r, g, b), fuzz);
    }
    else if (c1 == 'D') {
        double index;
        fscanf(file, "%lf)", &index);
        return make_shared<dielectric>(index);
    }

    throw std::invalid_argument("Could not parse Material!");
}

const shared_ptr<model> parse_model(FILE* file, const char* mode = "brute") {
    std::clog << "\rLoading Scene (Building Model)...           " << std::flush;
    char model_path[128];
    fscanf(file, "%s ", model_path);
    shared_ptr<material> mat = parse_material(file);
    return make_shared<model>(model_path, mat, mode);
}

const shared_ptr<sphere> parse_sphere(FILE* file) {
    std::clog << "\rLoading Scene (Building Sphere)...          " << std::flush;
    double x, y, z;
    double radius;
    fscanf(file, " (%lf %lf %lf) %lf ", &x, &y, &z, &radius);
    shared_ptr<material> mat = parse_material(file);
    return make_shared<sphere>(point(x, y, z), radius, mat);
}

const shared_ptr<quad> parse_quad(FILE* file) {
    std::clog << "\rLoading Scene (Building Quad)...            " << std::flush;
    double qx, qy, qz;
    double ux, uy, uz;
    double vx, vy, vz;
    fscanf(file, " (%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf) ",
            &qx, &qy, &qz, &ux, &uy, &uz, &vx, &vy, &vz);
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
    std::clog << "size:" << std::endl;
    std::clog << world.objects.size() << std::endl;

    if (strcmp(mode, "bvh") == 0)
        world = hittable_list(make_shared<bvh_node>(world));
    else if (strcmp(mode, "kd") == 0)
        world = hittable_list(make_shared<kd_node>(world));
    else if (strcmp(mode, "bih") == 0)
        world = hittable_list(make_shared<bih_node>(world));

    return world;
}

#endif
