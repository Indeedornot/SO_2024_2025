#include "SharedData.h"
#include <stdexcept>
#include "./Globals.h"

int SharedData::get_producer_value(const int producer_id) const {
    if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
        throw std::out_of_range("Invalid producer ID");
    }
    return producer_values[producer_id - 1];
}

void SharedData::set_producer_value(const int producer_id, const int value) {
    if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
        throw std::out_of_range("Invalid producer ID");
    }
    producer_values[producer_id - 1] = value;
}

void SharedData::increment_producer_value(const int producer_id, const int increment) {
    if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
        throw std::out_of_range("Invalid producer ID");
    }
    producer_values[producer_id - 1] += increment;
}

void SharedData::decrement_producer_value(const int producer_id, const int decrement) {
    if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
        throw std::out_of_range("Invalid producer ID");
    }
    producer_values[producer_id - 1] -= decrement;
}