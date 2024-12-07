#include "color.h"
#include "common.h"
#include <iostream>
#include <ostream>
#include <vector>

class stat_collector {
private:
    unsigned int pixel_index = 0;
    unsigned int sample_index = 0;
public:
    unsigned int samples_per_pixel;
    std::vector<int> n_intersection_tests;
    std::vector<int> n_traversal_steps;
    std::vector<int> sample_indeces;
    std::vector<int> pixel_indeces;
    bool freeze = false;
    stat_collector(unsigned int p_samples_per_pixel = 1) : n_intersection_tests(), n_traversal_steps(), sample_indeces(), pixel_indeces() {
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
    void save_csv(std::string name){
        std::clog << "\rSaving to CSV...                         " << std::flush;
        std::ofstream stream(("output/stats/" + name + "_stats.csv"));
        stream << "pixel index,sample index,number of traversal steps,number of intersection tests,\n";
        for (int i = 0; i < n_intersection_tests.size(); i++){
            stream << pixel_indeces[i] << ",";
            stream << sample_indeces[i] << ",";
            stream << n_traversal_steps[i] << ",";
            stream << n_intersection_tests[i] << ",\n";
        }
        stream.close();
        std::clog << "\rSaved to CSV!                       " << std::endl;
    }


    void plot_data(const std::vector<int> data, int width, int height, const color c, std::string name){
        int min = min_value(data);
        float range = max_value(data) - min;
        std::ofstream image(name);

        image << "P3\n" << width << ' ' << height << "\n255\n";
        int pixels = data.size() / samples_per_pixel;

        for (int i = 0; i < pixels; i++){
            std::clog << "\rProgress: " << std::flush;
            int ratio = (double(i) / pixels) * 20;
            std::clog << '[' << std::string(ratio, '#') << std::string(20-ratio, '-') << "] " << std::flush;
            float data_point = 0;
            for (int j = 0; j < samples_per_pixel; j++){
                data_point += ((data[i*samples_per_pixel+j] - min) * (1.0f / range));
            }
            write_color(image, c * (data_point / samples_per_pixel));
        }
        image.close();

    }

    void save_traversal_step_image(std::string name, int width, int height){
        std::clog << "\rCollecting Traversals...                     " << std::flush;
        plot_data(
            n_traversal_steps, 
            width, height, 
            color(1.0,1.0,1.0), 
            "output/stats/" + name + "_traversals.ppm"
        );
        std::clog << "\rTraversals Saved!                                " << std::endl;
    }

    void save_intersection_tests_image(std::string name, int width, int height){
        std::clog << "\rCollecting Intersections...                  " << std::flush;
        plot_data(
            n_intersection_tests, 
            width, height, 
            color(1.0,0.0,0.0), 
            "output/stats/" + name + "_intersections.ppm"
        );
        std::clog << "\rIntersections Saved!                                " << std::endl;
    }

    std::string get_file_name(std::string path) {
        auto base_file = path.substr(path.find_last_of("/\\") + 1);
        std::string::size_type const p(base_file.find_last_of('.'));
        return base_file.substr(0, p);
    }
};