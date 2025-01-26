#include "Director.h"
#include "../SharedData/Globals.h"
#include "../Managers/SemaphoreManager.h"
#include "../Logger/Logger.h"
#include <atomic>
#include <iostream>

Director::Director(SharedData *shared_data) : shared_data(shared_data) {
    receiver_mutex = SemaphoreManager::create_semaphore(SEM_RECEIVER_MUTEX, 1, "Director");
    logger.log(Logger::DIRECTOR, "Director started.");
}

Director::~Director() {
    SemaphoreManager::close_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
    logger.log(Logger::DIRECTOR, "Director cleaned up.");
}

void Director::run() const {
    std::string command;
    while (!shared_data->stop_signal.load()) {
        std::cout << "Enter command (save, quit, status): ";
        std::cin >> command;

        if (command == "save") {
            stop_processes(true);
        } else if (command == "quit") {
            stop_processes(false);
        } else if (command == "status") {
            print_status();
        } else {
            std::cout << "Invalid command. Try again." << std::endl;
        }
    }
}

void Director::stop_processes(const bool save) const {
    shared_data->stop_signal.store(true);
    shared_data->save_state.store(save);
    std::cout << "Stopping all processes " << (save ? "with" : "without") << " saving." << std::endl;
}

void Director::print_status() const {
    SemaphoreManager::lock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
    for (int i = 1; i <= MAX_PRODUCERS; ++i) {
        std::cout << "Producer " << i << " has total value: " << shared_data->get_producer_value(i) << std::endl;
    }
    SemaphoreManager::unlock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
}