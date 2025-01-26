#include "Director.h"
#include "../SharedData/Globals.h"
#include "../Managers/SemaphoreManager.h"
#include "../Logger/Logger.h"
#include <atomic>
#include <iostream>

Director::Director(SharedData *shared_data, std::vector<int> producer_pids, std::vector<int> receiver_pids) :
   shared_data(shared_data) ,
   producer_pids(producer_pids),
   receiver_pids(receiver_pids)
{
    receiver_mutex = SemaphoreManager::create_semaphore(SEM_RECEIVER_MUTEX, 1, "Director");
    logger.log(Logger::DIRECTOR, "Director started.");
}

Director::~Director() {
    SemaphoreManager::close_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
    logger.log(Logger::DIRECTOR, "Director cleaned up.");
}

void Director::run() const {
    std::string command;
    while (true) {
        std::cout << "Enter command (stop_producers, stop_receivers, save, quit, status): ";
        std::cin >> command;

        if (command == "save") {
            stop_processes(true);
            break;
        } else if (command == "quit") {
            stop_processes(false);
            break;
        } else if (command == "status") {
            print_status();
        } else if (command == "stop_producers") {
            stop_producers();
        } else if (command == "stop_receivers") {
            stop_receivers();
        } else {
            std::cout << "Invalid command. Try again." << std::endl;
        }
    }
}

void Director::stop_processes(const bool save) const {
    int signal = save ? STOP_ALL_WITH_SAVE_SIGNAL : STOP_ALL_SIGNAL;
    for (const auto pid: producer_pids) {
        if(kill(pid, signal) == -1) {
            logger.perror(Logger::DIRECTOR, "Failed to send signal " + std::to_string(signal) + " to Producer " + std::to_string(pid));
        } else {
            logger.log(Logger::DIRECTOR, "Sent signal " + std::to_string(signal) + " to Producer " + std::to_string(pid));
        }
    }

    for (const auto pid: receiver_pids) {
        if(kill(pid, signal) == -1) {
            logger.perror(Logger::DIRECTOR, "Failed to send signal " + std::to_string(signal) + " to Receiver " + std::to_string(pid));
        } else {
            logger.log(Logger::DIRECTOR, "Sent signal " + std::to_string(signal) + " to Receiver " + std::to_string(pid));
        }
    }
    std::cout << "Stopping all processes " << (save ? "with" : "without") << " saving." << std::endl;
}

void Director::stop_producers() const {
    for (const auto pid: producer_pids) {
        if(kill(pid, STOP_PRODUCER_SIGNAL) == -1) {
            logger.perror(Logger::DIRECTOR, "Failed to send signal " + std::to_string(STOP_PRODUCER_SIGNAL) + " to Producer " + std::to_string(pid));
        } else {
            logger.log(Logger::DIRECTOR, "Sent signal " + std::to_string(STOP_PRODUCER_SIGNAL) + " to Producer " + std::to_string(pid));
        }
    }

    std::cout << "Stopping all producers." << std::endl;
}

void Director::stop_receivers() const {
    for (const auto pid: receiver_pids) {
        if(kill(pid, STOP_RECEIVER_SIGNAL) == -1) {
          logger.perror(Logger::DIRECTOR, "Failed to send signal " + std::to_string(STOP_RECEIVER_SIGNAL) + " to Receiver " + std::to_string(pid));
        } else {
            logger.log(Logger::DIRECTOR, "Sent signal " + std::to_string(STOP_RECEIVER_SIGNAL) + " to Receiver " + std::to_string(pid));
        }
    }

    std::cout << "Stopping all receivers." << std::endl;
}

void Director::print_status() const {
    SemaphoreManager::lock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
    for (int i = 1; i <= MAX_PRODUCERS; ++i) {
        std::cout << "Producer " << i << " has total value: " << shared_data->get_producer_value(i) << std::endl;
    }
    SemaphoreManager::unlock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
}