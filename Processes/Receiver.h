#ifndef RECEIVER_H
#define RECEIVER_H

#include "../SharedData/SharedData.h"
#include <map>
#include <string>
#include <semaphore.h>

class Receiver {
public:
    Receiver(int id, const std::string& name, std::map<int, int> assigned_producers, SharedData *shared_data);
    ~Receiver();

    void run();
    void stop(int signal);

private:
    int id;
    std::string name;
    std::map<int, int> assigned_producers;
    SharedData *shared_data;
    sem_t *receiver_mutex;
    bool running = true;
};

#endif // RECEIVER_H