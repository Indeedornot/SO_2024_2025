#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <map>
#include <random>
#include <thread>
#include <fstream>
#include <sys/wait.h>
#include <string>
#include <filesystem>

#include "./Logger/Logger.h"
#include "./SharedData/SharedData.h"
#include "./SharedData/SharedDataManager.h"
#include "./Managers/SemaphoreManager.h"
#include "./Processes/Director.h"
#include "./Processes/Producer.h"
#include "./Processes/Receiver.h"

void create_producers(const std::map<int, std::tuple<std::string, int, int>> &producers, SharedData *shared_data) {
    for (const auto &[producer_id, params]: producers) {
        auto [name, value_per_cycle, limit] = params;
        if (fork() == 0) {
            const Producer producer(producer_id, name, value_per_cycle, limit, shared_data);
            producer.run();
            exit(0);
        }
    }
}

void create_receivers(const std::map<int, std::tuple<std::string, std::map<int, int>>> &receiver_configs, SharedData *shared_data) {
    for (const auto &[receiver_id, params]: receiver_configs) {
        auto [name, assigned_producers] = params;
        if (fork() == 0) {
            const Receiver receiver(receiver_id, name, assigned_producers, shared_data);
            receiver.run();
            exit(0);
        }
    }
}

int main() {
    SemaphoreManager::clear_stale_semaphores();
    const SharedDataManager shared_data_manager;
    SharedData *shared_data = shared_data_manager.get_shared_data();

    // Producer ID -> {Name, Value per cycle, Limit}
    const std::map<int, std::tuple<std::string, int, int>> producers = {
        {1, {"ProducerA", 5, 20}}, {2, {"ProducerB", 10, 30}}, {3, {"ProducerC", 3, 10}}
    };

    // Receiver ID -> {Name, {Producer ID -> Consume amount}}
    const std::map<int, std::tuple<std::string, std::map<int, int>>> receiver_configs = {
        {1, {"ReceiverX", {{1, 2}, {2, 5}}}},
        {2, {"ReceiverY", {{2, 3}, {3, 1}}}}
    };

    create_producers(producers, shared_data);
    create_receivers(receiver_configs, shared_data);

    Director director(shared_data);
    std::thread director_thread(&Director::run, &director);

    while (true) {
        int status;
        if (wait(&status) == -1) {
            if (errno == ECHILD) {
                std::cout << "All child processes finished." << std::endl;
                break;
            }
            perror("wait");
            exit(EXIT_FAILURE);
        }
    }

    director_thread.join();
    return 0;
}