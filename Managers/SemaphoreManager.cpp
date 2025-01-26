#include "SemaphoreManager.h"
#include "../SharedData/Globals.h"
#include <fcntl.h>
#include <string.h>

sem_t *SemaphoreManager::create_semaphore(const std::string &name, const int value, const std::string &owner) {
    sem_t *sem = sem_open(name.c_str(), O_CREAT, 0600, value);
    if (sem == SEM_FAILED) {
        logger.perror(Logger::GENERAL, owner + " failed to create semaphore: " + name);
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::GENERAL, owner + " created semaphore: " + name);
    return sem;
}

void SemaphoreManager::lock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    logger.log(Logger::LOCK_SEMAPHORE, owner + " attempting to lock semaphore: " + name);
    if (sem_wait(sem) == -1) {
        logger.perror(Logger::LOCK_SEMAPHORE, owner + " failed to lock semaphore: " + name);
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::LOCK_SEMAPHORE, owner + " locked semaphore: " + name);
}

void SemaphoreManager::unlock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    logger.log(Logger::UNLOCK_SEMAPHORE, owner + " attempting to unlock semaphore: " + name);
    if (sem_post(sem) == -1) {
        logger.perror(Logger::UNLOCK_SEMAPHORE, owner + " failed to unlock semaphore: " + name);
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::UNLOCK_SEMAPHORE, owner + " unlocked semaphore: " + name);
}

void SemaphoreManager::close_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
    if(sem_close(sem) == -1) {
        logger.perror(Logger::GENERAL, owner + " failed to close semaphore: " + name);
        exit(EXIT_FAILURE);
    }

    if(sem_unlink(name.c_str()) == -1) {
        logger.perror(Logger::GENERAL, owner + " failed to unlink semaphore: " + name);
        exit(EXIT_FAILURE);
    }
    logger.log(Logger::GENERAL, owner + " closed semaphore: " + name);
}

// Clear stale semaphores - e.g. if there was a forced stop
// This is only a precaution and should not be necessary
void SemaphoreManager::clear_stale_semaphores() {
    for (int i = 0; i <= MAX_PRODUCERS; ++i) {
        std::string sem_name = std::string(SEM_PRODUCER_PREFIX) + std::to_string(i);
        if(sem_unlink(sem_name.c_str()) == -1) {
            logger.perror(Logger::GENERAL, "Failed to unlink AN OLD semaphore: " + sem_name);
        }
    }

    if(sem_unlink(SEM_RECEIVER_MUTEX) == -1) {
        logger.perror(Logger::GENERAL, "Failed to unlink AN OLD semaphore: " + std::string(SEM_RECEIVER_MUTEX));
    }
    if(sem_unlink(SEM_GLOBAL_MUTEX) == -1) {
        logger.perror(Logger::GENERAL, "Failed to unlink AN OLD semaphore: " + std::string(SEM_GLOBAL_MUTEX));
    }
}