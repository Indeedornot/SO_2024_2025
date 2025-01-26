#ifndef DIRECTOR_H
#define DIRECTOR_H

#include "../SharedData/SharedData.h"
#include <semaphore.h>
#include <vector>

class Director {
public:
    explicit Director(SharedData *shared_data, std::vector<int> producer_pids, std::vector<int> receiver_pids);
    ~Director();

    void run() const;

private:
    SharedData *shared_data;
    sem_t *receiver_mutex;

    void stop_processes(bool save) const;
    void print_status() const;
    void stop_producers() const;
    void stop_receivers() const;

    std::vector<int> producer_pids;
    std::vector<int> receiver_pids;
};

#endif // DIRECTOR_H