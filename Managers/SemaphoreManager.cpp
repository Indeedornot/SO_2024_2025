#include "SemaphoreManager.h"
#include "../SharedData/Globals.h"
#include <fcntl.h>

sem_t *SemaphoreManager::create_semaphore(const std::string &name, const int value, const std::string &owner) {
    sem_t *sem = sem_open(name.c_str(), O_CREAT, 0666, value);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::GENERAL, owner + " created semaphore: " + name);
    return sem;
}

void SemaphoreManager::lock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    logger.log(Logger::LOCK_SEMAPHORE, owner + " attempting to lock semaphore: " + name);
    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::LOCK_SEMAPHORE, owner + " locked semaphore: " + name);
}

void SemaphoreManager::unlock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    logger.log(Logger::UNLOCK_SEMAPHORE, owner + " attempting to unlock semaphore: " + name);
    if (sem_post(sem) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::UNLOCK_SEMAPHORE, owner + " unlocked semaphore: " + name);
}

void SemaphoreManager::close_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    sem_close(sem);
    sem_unlink(name.c_str());
    logger.log(Logger::GENERAL, owner + " closed semaphore: " + name);
}

// Clear stale semaphores - e.g. if there was a forced stop
void SemaphoreManager::clear_stale_semaphores() {
    for (int i = 0; i <= MAX_PRODUCERS; ++i) {
        std::string sem_name = std::string(SEM_PRODUCER_PREFIX) + std::to_string(i);
        sem_unlink(sem_name.c_str());
    }
    sem_unlink(SEM_RECEIVER_MUTEX);
    sem_unlink(SEM_GLOBAL_MUTEX);
}