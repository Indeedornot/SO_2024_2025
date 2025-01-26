#include "SharedDataManager.h"
#include "../Logger/Logger.h"
#include "../Managers/SemaphoreManager.h"
#include "./SharedData.h"
#include "./Globals.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>
#include <stdexcept>

SharedDataManager::SharedDataManager() {
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        logger.perror(Logger::SHARED_MEMORY, "Failed to create shared memory.");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shm_fd, SHM_SIZE) == -1) {
        logger.perror(Logger::SHARED_MEMORY, "Failed to truncate shared memory.");
        exit(EXIT_FAILURE);
    }
    shared_data = static_cast<SharedData *>(mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shared_data == MAP_FAILED) {
        logger.perror(Logger::SHARED_MEMORY, "Failed to map shared memory.");
        exit(EXIT_FAILURE);
    }
    new(shared_data) SharedData();
    shared_data->global_mutex = SemaphoreManager::create_semaphore(SEM_GLOBAL_MUTEX, 1, "SharedDataManager");
    std::ranges::fill(shared_data->producer_values, 0);
    logger.log(Logger::SHARED_MEMORY, "Shared memory initialized.");
}

SharedDataManager::~SharedDataManager() {
    shared_data->~SharedData();
    SemaphoreManager::close_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "SharedDataManager");

    if (munmap(shared_data, SHM_SIZE) == -1) {
        logger.perror(Logger::SHARED_MEMORY, "munmap failed");
    }
    if (close(shm_fd) == -1) {
        logger.perror(Logger::SHARED_MEMORY, "close(shm_fd) failed");
    }
    if (shm_unlink(SHM_NAME) == -1) {
        logger.perror(Logger::SHARED_MEMORY, "shm_unlink failed");
    }
    logger.log(Logger::SHARED_MEMORY, "Shared memory cleaned up.");
}

SharedData* SharedDataManager::get_shared_data() const { return shared_data; }
