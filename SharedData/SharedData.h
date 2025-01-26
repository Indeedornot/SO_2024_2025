#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <atomic>
#include <semaphore.h>
#include "../SharedData/Globals.h"

struct SharedData {
    int producer_values[MAX_PRODUCERS]{};
    sem_t *global_mutex{};

    [[nodiscard]] int get_producer_value(int producer_id) const;
    void set_producer_value(int producer_id, int value);
    void increment_producer_value(int producer_id, int increment);
    void decrement_producer_value(int producer_id, int decrement);
};

#endif //SHARED_DATA_H