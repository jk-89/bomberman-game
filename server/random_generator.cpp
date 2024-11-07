#include "random_generator.h"

RandomGenerator::RandomGenerator(uint32_t seed) {
    random = std::make_shared<std::minstd_rand>(seed);
}

// Default seed.
RandomGenerator::RandomGenerator() :
        RandomGenerator((uint32_t ) std::chrono::system_clock::now().time_since_epoch().count()) {}

void RandomGenerator::set_sizes(coordinate_t size_x_, coordinate_t size_y_) {
    size_x = size_x_;
    size_y = size_y_;
}

coordinate_t RandomGenerator::rand_x() {
    return (coordinate_t) ((*random)() % size_x);
}

coordinate_t RandomGenerator::rand_y() {
    return (coordinate_t) ((*random)() % size_y);
}


