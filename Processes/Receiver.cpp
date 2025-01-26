#include "Receiver.h"
#include "../SharedData/Globals.h"
#include "../Managers/SemaphoreManager.h"
#include "../Logger/Logger.h"
#include "../Managers/SleepManager.h"
#include "../Managers/RandomManager.h"
#include <algorithm>

Receiver::Receiver(const int id, const std::string& name, std::map<int, int> assigned_producers, SharedData *shared_data)
    : id(id), name(name), assigned_producers(std::move(assigned_producers)), shared_data(shared_data) {
    receiver_mutex = SemaphoreManager::create_semaphore(SEM_RECEIVER_MUTEX, 1, "Receiver " + name);
    logger.log(Logger::RECEIVER, "Receiver " + name + " started.");
}

Receiver::~Receiver() {
    SemaphoreManager::close_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Receiver " + name);
    logger.log(Logger::RECEIVER, "Receiver " + name + " cleaned up.");
}

void Receiver::run() {
    RandomManager randomManager;
    while (running) {
        SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver " + name);

        const bool enough_values = std::ranges::all_of(assigned_producers, [this](const auto &pair) {
            auto [producer_id, consume_amount] = pair;
            return shared_data->get_producer_value(producer_id) >= consume_amount;
        });

        if (!enough_values) {
            SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver " + name);
            logger.log(Logger::RECEIVER, "Receiver " + name + " did not find enough values to consume. Waiting.");
            SleepManager::sleep_ms(randomManager.get_random_int(500, 2000));
            continue;
        }

        for (const auto &[producer_id, consume_amount]: assigned_producers) {
            shared_data->decrement_producer_value(producer_id, consume_amount);
            logger.log(Logger::RECEIVER, "Receiver " + name + " consumed " + std::to_string(consume_amount) + " from Producer " + std::to_string(producer_id) + ". Remaining: " + std::to_string(shared_data->get_producer_value(producer_id)));
        }

        SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver " + name);
        SleepManager::sleep_ms(randomManager.get_random_int(500, 2000));
    }
}

void Receiver::stop(int signal) {
    logger.log(Logger::RECEIVER, "Receiver " + name + " received " + std::to_string(signal) + " stop signal.");
    running = false;
}