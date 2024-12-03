#include "common.h"
#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "quad.h"
#include "mesh.h"
#include "model.h"

void random_spheres() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point(4, 1, 0), 1.0, material3));

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.width             = 400;
    cam.samples_per_pixel = 50;
    cam.max_depth         = 10;

    cam.vfov     = 20;
    cam.lookfrom = point(13,2,3);
    cam.lookat   = point(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}

void quads() {
    hittable_list world;

    auto left_red = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    world.add(make_shared<triangle>(point(-3, -2, 5), vec3(0, 0, -4), vec3(0, 4, 0), left_red));
    world.add(make_shared<triangle>(point(-2, -2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<triangle>(point(3, -2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<triangle>(point(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<triangle>(point(-2, -3, 5), vec3(4, 0, 0), vec3(0, 0, -4), lower_teal));

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.width = 400;
    cam.samples_per_pixel = 100;
    cam.stats->samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vfov = 80;
    cam.lookfrom = point(0,0,9);
    cam.lookat = point(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void triangles() {
    hittable_list world;

    auto material = make_shared<lambertian>(color(1.0, 0.2, 0.0));

    world.add(make_shared<triangle>(point(-1, -2, 0), point(-2, 0, 0), point(2, 3, -1), material));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.width = 400;
    cam.samples_per_pixel = 100;
    cam.stats->samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vfov = 80;
    cam.lookfrom = point(0,0,9);
    cam.lookat = point(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void meshes() {
    hittable_list world;

    auto material = make_shared<lambertian>(color(1.0, 0.2, 0.2));

    std::vector<vec3> vertices = { vec3(-2, -2, 0), vec3(2, -2, 0), vec3(1, -2, 2), vec3(0, 0, 1) };
    std::vector<vec3> face_indices = { vec3(0, 1, 2), vec3(0, 4, 2), vec3(1, 4, 0), vec3(2, 4, 1) };

    world.add(make_shared<mesh>(vertices, face_indices, material));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.width = 400;
    cam.samples_per_pixel = 1;
    cam.stats->samples_per_pixel = 1;
    cam.max_depth = 50;

    cam.vfov = 80;
    cam.lookfrom = point(0,0,9);
    cam.lookat = point(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void test_model() {
    hittable_list world;

    auto material = make_shared<metal>(color(0.8, 0.5, 0.2), 0.1);

    world.add(make_shared<model>("bunny.obj", material));
    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.width = 400;
    cam.samples_per_pixel = 1;
    cam.stats->samples_per_pixel = 1;
    cam.max_depth = 20;

    cam.vfov = 90;
    cam.lookfrom = point(-0.05, 0.15, 0.1);
    cam.lookat = point(0, 0.1, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}

int main()
{
    switch(4) {
        case 0: random_spheres(); break;
        case 1: quads(); break;
        case 2: triangles(); break;
        case 3: meshes(); break;
        case 4: test_model(); break;
    }

    return 0;
}
