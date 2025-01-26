#ifndef PRODUCER_H
#define PRODUCER_H

#include "../SharedData/SharedData.h"
#include <string>
#include <semaphore.h>

namespace fs = std::filesystem;

class Producer {
public:
    Producer(int id, const std::string& name, int value_per_cycle, int limit, SharedData *shared_data);
    ~Producer();

    void run() const;

private:
    int id;
    std::string name;
    int value_per_cycle;
    int limit;
    SharedData *shared_data;
    std::string sem_name;
    sem_t *sem;
};

#endif // PRODUCER_H