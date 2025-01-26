#ifndef DIRECTOR_H
#define DIRECTOR_H

#include "../SharedData/SharedData.h"
#include <semaphore.h>

class Director {
public:
    explicit Director(SharedData *shared_data);
    ~Director();

    void run() const;

private:
    SharedData *shared_data;
    sem_t *receiver_mutex;

    void stop_processes(bool save) const;
    void print_status() const;
};

#endif // DIRECTOR_H