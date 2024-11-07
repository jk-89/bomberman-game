// Class used to handle randomized variables inside server.

#ifndef ZAD2_RANDOM_GENERATOR_H
#define ZAD2_RANDOM_GENERATOR_H

#include <iostream>
#include <random>
#include <cstdint>
#include <chrono>
#include "utils.h"
#include "map_objects.h"

class RandomGenerator {
private:
    std::shared_ptr<std::minstd_rand> random;
    coordinate_t size_x{}, size_y{};

public:
    explicit RandomGenerator(uint32_t seed);
    RandomGenerator();

    void set_sizes(coordinate_t size_x_, coordinate_t size_y_);

    coordinate_t rand_x();
    coordinate_t rand_y();
};


#endif //ZAD2_RANDOM_GENERATOR_H
