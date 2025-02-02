#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <algorithm>
#include <fstream>
#include <vector>

using std::make_shared;
using std::shared_ptr;

const double infinity  = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radian(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    return min + (max-min) * random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max+1));
}

inline int min_value(const std::vector<int> v) {
        int min = 999999999;
        for (int i: v){
            if (i < min) {
                min = i;
            }
        }
        return min;
}

inline int max_value(const std::vector<int> v) {
        int max = -999999999;
        for (int i: v){
            if (i > max) {
                max = i;
            }
        }
        return max;
}

#include "geometry.h"
#include "color.h"
#include "parser.h"

#endif
