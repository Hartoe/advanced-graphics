#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "hittable.h"
#include "material.h"

class camera {
    private:
        double pixel_sample_scale;
        point center;
        point pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        vec3 u, v, w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;


        ray get_ray(int x, int y) const {
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((x + offset.x()) * pixel_delta_u) + ((y + offset.y()) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 sample_square() const {
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

        point defocus_disk_sample() const {
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            if (depth <= 0)
                return color(0,0,0);

            hit_record rec;
            rec.stats = stats;

            if (world.hit(r, interval(0.0001, infinity), rec)) {
                rec.stats->freeze = true;
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered))
                    return attenuation * ray_color(scattered, depth-1, world);
                return color(0,0,0);
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y() + 1.0);
            return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
        }
    public:
        double aspect_ratio = 1.0;
        int width = 100;
        int height;
        int samples_per_pixel = 1;
        int max_depth = 10;
        double vfov = 90;
        point lookfrom = point(0, 0, 0);
        point lookat = point(0, 0, -1);
        vec3 vup = point(0, 1, 0);
        double defocus_angle = 0;
        double focus_dist = 10;
        std::shared_ptr<stat_collector> stats;
        void initialize() {           
            height = int(width / aspect_ratio);
            height = (height < 1) ? 1 : height;

            pixel_sample_scale = 1.0 / samples_per_pixel;
            stats = std::make_shared<stat_collector>(stat_collector(samples_per_pixel));
            stats->samples_per_pixel = samples_per_pixel;

            center = lookfrom;

            auto theta = degrees_to_radian(vfov);
            auto h = std::tan(theta/2);
            auto viewport_height = 2 * h * focus_dist;
            auto viewport_width = viewport_height * (double(width)/height);

            w = unit_vector(lookfrom- lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            auto viewport_u = viewport_width * u;
            auto viewport_v = viewport_height * -v;

            pixel_delta_u = viewport_u / width;
            pixel_delta_v = viewport_v / height;

            auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 -viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            auto defocus_radius = focus_dist * std::tan(degrees_to_radian(defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        void render(const hittable& world, const char* path = "image.ppm") {
            initialize();

            std::ofstream image(path);

            image << "P3\n" << width << ' ' << height << "\n255\n";

            for (int y = 0; y < height; y++)
            {
                print_loading(y);
                for (int x = 0; x < width; x++)
                {
                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; sample++)
                    {
                        stats->new_row();
                        ray r = get_ray(x, y);
                        pixel_color += ray_color(r, max_depth, world);
                        stats->next_sample();
                    }
                    
                    stats->next_pixel();
                    write_color(image, pixel_sample_scale * pixel_color);
                }
            }
            image.close();
        }

        void print_loading(int progress) {
            std::clog << "\rCurrent Line: " << progress << '/' << height << ' ';
            int ratio = (double(progress) / height) * 20;
            std::clog << '[' << std::string(ratio, '#') << std::string(20-ratio, '-') << "] " << std::flush;
        }

        void save_stats(std::string path) {
            std::clog << "\rCollecting Stats...                             " << std::flush;
            auto file_name = stats->get_file_name(path);
            stats->save_csv(file_name);
            stats->save_intersection_tests_image(file_name, width, height);
            stats->save_traversal_step_image(file_name, width, height);
        }
};

#endif
