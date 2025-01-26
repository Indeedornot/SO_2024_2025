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
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    shared_data = static_cast<SharedData *>(mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    new(shared_data) SharedData();
    shared_data->global_mutex = SemaphoreManager::create_semaphore(SEM_GLOBAL_MUTEX, 1, "SharedDataManager");
    std::ranges::fill(shared_data->producer_values, 0);
    logger.log(Logger::SHARED_MEMORY, "Shared memory initialized.");
}

SharedDataManager::~SharedDataManager() {
    shared_data->~SharedData();
    SemaphoreManager::close_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "SharedDataManager");
    munmap(shared_data, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    logger.log(Logger::SHARED_MEMORY, "Shared memory cleaned up.");
}

SharedData* SharedDataManager::get_shared_data() const { return shared_data; }
