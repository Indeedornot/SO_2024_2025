#ifndef SEMAPHOREMANAGER_H
#define SEMAPHOREMANAGER_H

#include <semaphore.h>
#include <string>
#include <iostream>

class SemaphoreManager {
public:
    static sem_t *create_semaphore(const std::string &name, int value, const std::string &owner);
    static void lock_semaphore(sem_t *sem, const std::string &name, const std::string &owner);
    static void unlock_semaphore(sem_t *sem, const std::string &name, const std::string &owner);
    static void close_semaphore(sem_t *sem, const std::string &name, const std::string &owner);
    static void clear_stale_semaphores();
};

#endif // SEMAPHOREMANAGER_H