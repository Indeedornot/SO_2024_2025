//
// Created by indeed on 1/26/25.
//

#include "RandomManager.h"
#include <stdexcept>

int RandomManager::get_random_int(int min, int max) {
    if(min > max) {
        throw std::invalid_argument("Min value cannot be greater than max value.");
    }

    if(dist.min() != min || dist.max() != max) {
        dist = std::uniform_int_distribution<int>(min, max);
    }
    return dist(gen);
}

RandomManager::RandomManager() : gen(rd()) {}