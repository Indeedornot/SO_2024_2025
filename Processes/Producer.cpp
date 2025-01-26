#include "Producer.h"
#include "../SharedData/Globals.h"
#include "../Managers/SemaphoreManager.h"
#include "../Logger/Logger.h"
#include "../Managers/SleepManager.h"
#include "../Managers/RandomManager.h"
#include <atomic>
#include <random>
#include <filesystem>
#include <fstream>
#include <signal.h>

Producer::Producer(const int id, const std::string& name, const int value_per_cycle, const int limit, SharedData *shared_data)
    : id(id), name(name), value_per_cycle(value_per_cycle), limit(limit), shared_data(shared_data) {

    sem_name = std::string(SEM_PRODUCER_PREFIX) + std::to_string(id);
    sem = SemaphoreManager::create_semaphore(sem_name, 1, "Producer " + name);
    logger.log(Logger::PRODUCER, "Producer " + name + " started with limit " + std::to_string(limit));
}

Producer::~Producer() {
    SemaphoreManager::close_semaphore(sem, sem_name, "Producer " + name);
    logger.log(Logger::PRODUCER, "Producer " + name + " cleaned up.");
}

void Producer::run() const {
    RandomManager randomManager;

    auto file_path = fs::path("producer_" + name + ".txt");
    if (!fs::exists(file_path)) {
        std::ofstream file(file_path);
        file << 0 << std::endl;
    } else {
        logger.log(Logger::PRODUCER, "Producer " + name + " found previous value file.");
        std::ifstream file(file_path);
        int value;
        file >> value;

        SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);
        shared_data->set_producer_value(id, value);
        SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);

        logger.log(Logger::PRODUCER, "Producer " + name + " loaded previous value: " + std::to_string(value));
        fs::remove(file_path);
        logger.log(Logger::PRODUCER, "Producer " + name + " removed previous value file.");
    }

    while (running) {
        SleepManager::sleep_ms(randomManager.get_random_int(500, 2000));

        SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);

        if (shared_data->get_producer_value(id) + value_per_cycle <= limit) {
            shared_data->increment_producer_value(id, value_per_cycle);
            logger.log(Logger::PRODUCER, "Producer " + name + " produced " + std::to_string(value_per_cycle) + ". Total: " + std::to_string(shared_data->get_producer_value(id)));
        } else {
            logger.log(Logger::PRODUCER, "Producer " + name + " reached its limit. Skipping production.");
        }

        SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);
    }

    if (save_state) {
        std::ofstream file(file_path);
        file << shared_data->get_producer_value(id) << std::endl;
    }
}

void Producer::stop(int signal) {
    logger.log(Logger::PRODUCER, "Producer " + name + " received " + std::to_string(signal) + " stop signal.");
    save_state = signal == STOP_ALL_WITH_SAVE_SIGNAL;
    running = false;
}
