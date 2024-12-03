#include "color.h"
#include <iostream>
#include <ostream>
#include <vector>

class stat_collector {
private:
    uint pixel_index = 0;
    uint sample_index = 0;
public:
    uint samples_per_pixel;
    std::vector<int> n_intersection_tests;
    std::vector<int> n_traversal_steps;
    std::vector<int> sample_indeces;
    std::vector<int> pixel_indeces;
    bool freeze = false;
    stat_collector(uint p_samples_per_pixel = 1) : n_intersection_tests(), n_traversal_steps(), sample_indeces(), pixel_indeces() {
        samples_per_pixel = p_samples_per_pixel;
    }
    void next_sample(){
        sample_index++;
    }
    void new_row(){
        n_traversal_steps.push_back(0);
        n_intersection_tests.push_back(0);
        sample_indeces.push_back(sample_index);
        pixel_indeces.push_back(pixel_index);
        freeze = false;
    }
    void next_pixel(){
        pixel_index++;
        sample_index = 0;
    }
    void record_traversal_step() {
        if (freeze) return;
        n_traversal_steps[pixel_index * samples_per_pixel + sample_index]++;
    }
    void record_intersection_test() {
        if (freeze) return;
        n_intersection_tests[pixel_index * samples_per_pixel + sample_index]++;
    }
    void print(){
    for (auto i: sample_indeces)
        std::cout << i << ' ';
    }
    void save_csv(){
        std::cout << "\n" << samples_per_pixel << std::endl;
        std::ofstream stream("stats.csv");
        stream << "pixel index,sample index,number of traversal steps,number of intersection tests,\n";
        for (int i = 0; i < n_intersection_tests.size(); i++){
            stream << pixel_indeces[i] << ",";
            stream << sample_indeces[i] << ",";
            stream << n_traversal_steps[i] << ",";
            stream << n_intersection_tests[i] << ",\n";
        }
        stream.close();
    }

    int maximum_intersections(){
        int max = 0;
        for (int i: n_intersection_tests){
            if (i > max) {
                max = i;
            }
        }
        return max;
    }
    int minimum_intersections(){
        int min = 999999999;
        for (int i: n_intersection_tests){
            if (i < min) {
                min = i;
            }
        }
        return min;
    }

    void save_intersection_tests_image(int width, int height){
        int min = minimum_intersections();
        float range = maximum_intersections() - min;
        std::ofstream image("intersections.ppm");

        image << "P3\n" << width << ' ' << height << "\n255\n";

        for (int i = 0; i < n_intersection_tests.size(); i++){
            int value = n_intersection_tests[i];
            float x = ((value - min) * (1.0f / range));
            color c(x, 0.0, 0.0);
            write_color(image, c);
        }
        image.close();
    }
    int maximum_traversals(){
        int max = 0;
        for (int i: n_traversal_steps){
            if (i > max) {
                max = i;
            }
        }
        return max;
    }
    int minimum_traversals(){
        int min = 999999999;
        for (int i: n_traversal_steps){
            if (i < min) {
                min = i;
            }
        }
        return min;
    }

    void save_traversal_step_image(int width, int height){
        int min = minimum_traversals();
        float range = maximum_traversals() - min;
        std::ofstream image("traversals.ppm");

        image << "P3\n" << width << ' ' << height << "\n255\n";

        for (int i = 0; i < n_traversal_steps.size(); i++){
            int value = n_traversal_steps[i];
            float x = ((value - min) * (1.0f / range));
            color c(x, x, x);
            write_color(image, c);
        }
        image.close();
    }
};
