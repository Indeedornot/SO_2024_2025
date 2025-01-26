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
#include <fcntl.h>

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

void Producer::load_previous_value() {
    auto file_path = fs::path("producer_" + name + ".txt");
    if (fs::exists(file_path)) {
        logger.log(Logger::PRODUCER, "Producer " + name + " found previous value file.");
        std::ifstream file(file_path);
        if (!file.is_open()) {
            logger.perror(Logger::PRODUCER, "Producer " + name + " failed to load previous value.");
            return;
        }
        int value;
        if(!(file >> value)) {
            logger.perror(Logger::PRODUCER, "Producer " + name + " failed to load previous value.");
            return;
        }

        SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);
        shared_data->set_producer_value(id, value);
        SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + name);

        logger.log(Logger::PRODUCER, "Producer " + name + " loaded previous value: " + std::to_string(value));
        if(!fs::remove(file_path)) {
            logger.perror(Logger::PRODUCER, "Producer " + name + " failed to remove previous value file.");
            return;
        }
        logger.log(Logger::PRODUCER, "Producer " + name + " removed previous value file.");
    } else {
        logger.log(Logger::PRODUCER, "Producer " + name + " did not find previous value file.");
    }
}

void Producer::run() {
    RandomManager randomManager;
    load_previous_value();

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

    logger.log(Logger::PRODUCER, "Producer " + name + " stopped.");

    if (save_state) {
        save_value();
    } else {
        logger.log(Logger::PRODUCER, "Producer " + name + " did not save value.");
    }
}

void Producer::save_value() {
    auto file_path = fs::path("producer_" + name + ".txt");

    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        logger.perror(Logger::PRODUCER, "Producer " + name + " failed to save value.");
        return;
    }
    if(close(fd) == -1) {
        logger.perror(Logger::PRODUCER, "Producer " + name + " failed to save value.");
        return;
    }
    std::ofstream file(file_path);
    if (file.is_open()) {
        file << shared_data->get_producer_value(id) << std::endl;
        logger.log(Logger::PRODUCER, "Producer " + name + " saved value: " + std::to_string(shared_data->get_producer_value(id)));
    } else {
        logger.perror(Logger::PRODUCER, "Producer " + name + " failed to save value.");
    }
}

void Producer::stop(int signal) {
    logger.log(Logger::PRODUCER, "Producer " + name + " received " + std::to_string(signal) + " stop signal.");
    save_state = signal == STOP_ALL_WITH_SAVE_SIGNAL;
    running = false;
}
