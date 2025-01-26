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

//Required to use an instance inside an interrupt handler
//A bit of a hack, but it works
static Producer *g_producer;
std::vector<int> create_producers(const std::map<int, std::tuple<std::string, int, int>> &producers, SharedData *shared_data) {
    std::vector<int> producer_pids;
    for (const auto &[producer_id, params]: producers) {
        auto [name, value_per_cycle, limit] = params;
        const int pid = fork();
        if (pid == 0) {
            Producer producer(producer_id, name, value_per_cycle, limit, shared_data);
            g_producer = &producer;
            signal(STOP_PRODUCER_SIGNAL, [](int sig) {
                g_producer->stop(sig);
            });

            signal(STOP_ALL_SIGNAL, [](int sig) {
                g_producer->stop(sig);
            });

            signal(STOP_ALL_WITH_SAVE_SIGNAL, [](int sig) {
                g_producer->stop(sig);
            });

            producer.run();
            exit(0);
        }
        if (pid == -1) {
            logger.perror(Logger::PRODUCER, "fork() failed for producer " + name);
            exit(EXIT_FAILURE);
        }
        producer_pids.push_back(pid);
    }

    return producer_pids;
}

//Required to use an instance inside an interrupt handler
//A bit of a hack, but it works
static Receiver *g_receiver;
std::vector<int> create_receivers(const std::map<int, std::tuple<std::string, std::map<int, int>>> &receiver_configs, SharedData *shared_data) {
    std::vector<int> receiver_pids;
    for (const auto &[receiver_id, params]: receiver_configs) {
        auto [name, assigned_producers] = params;
        const int pid = fork();
        if (pid == 0) {
            Receiver receiver(receiver_id, name, assigned_producers, shared_data);
            g_receiver = &receiver;
            signal(STOP_RECEIVER_SIGNAL, [](int sig) {
                    g_receiver->stop(sig);
                });

            signal(STOP_ALL_SIGNAL, [](int sig) {
                    g_receiver->stop(sig);
                });

            signal(STOP_ALL_WITH_SAVE_SIGNAL, [](int sig) {
                    g_receiver->stop(sig);
                });

            receiver.run();
            exit(0);
        }
        if (pid == -1) {
            logger.perror(Logger::RECEIVER, "fork() failed for receiver " + name);
            exit(EXIT_FAILURE);
        }
        receiver_pids.push_back(pid);
    }

    return receiver_pids;
}

int main() {
    SemaphoreManager::clear_stale_semaphores();
    const SharedDataManager shared_data_manager;
    SharedData *shared_data = shared_data_manager.get_shared_data();

    // Producer ID -> {Name, Value per cycle, Limit}
    const std::map<int, std::tuple<std::string, int, int>> producers = {
        {1, {"ProducerX", 1, 17}}, {2, {"ProducerY", 2, 33}}, {3, {"ProducerZ", 3, 50}}
    };

    // Receiver ID -> {Name, {Producer ID -> Consume amount}}
    const std::map<int, std::tuple<std::string, std::map<int, int>>> receiver_configs = {
        {1, {"ReceiverA", {{1, 1}, {2, 2}, {3, 3}}}},
        {2, {"ReceiverB", {{1, 1}, {2, 2}, {3, 3}}}}
    };

    std::vector<int> producer_pids = create_producers(producers, shared_data);
    std::vector<int> receiver_pids = create_receivers(receiver_configs, shared_data);

    Director director(shared_data, producer_pids, receiver_pids);
    std::thread director_thread(&Director::run, &director);

    while (true) {
        int status;
        if (wait(&status) == -1) {
            if (errno == ECHILD) {
                std::cout << "All child processes finished." << std::endl;
                break;
            }
            logger.perror(Logger::GENERAL, "wait() failed.");
            exit(EXIT_FAILURE);
        }
    }

    director_thread.join();
    return 0;
}