#include "Logger.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

Logger::Logger() : log_mode_(FILE_LOG) {
    enabled_categories_.insert({PRODUCER, RECEIVER, DIRECTOR, SHARED_MEMORY, GENERAL});
    if (log_mode_ & FILE_LOG) {
        const auto log_path = fs::path("log." + std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) + ".txt");
        log_file_.open(log_path);
        if (!log_file_) {
            std::cerr << "Failed to open log file." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

Logger::~Logger() {
    if (log_mode_ & FILE_LOG) {
        log_file_.close();
    }
}

void Logger::log(const Category category, const std::string &message) {
    if (enabled_categories_.contains(category)) {
        if (log_mode_ & CONSOLE_LOG) {
            std::cout << "[" << get_category_name(category) << "] " << message << std::endl;
        }

        if (log_mode_ & FILE_LOG) {
            log_file_ << "[" << get_category_name(category) << "] " << message << std::endl;
        }
    }
}

std::string Logger::get_category_name(const Category category) {
    switch (category) {
        case LOCK_SEMAPHORE: return "LOCK_SEMAPHORE";
        case UNLOCK_SEMAPHORE: return "UNLOCK_SEMAPHORE";
        case PRODUCER: return "PRODUCER";
        case RECEIVER: return "RECEIVER";
        case DIRECTOR: return "DIRECTOR";
        case SHARED_MEMORY: return "SHARED_MEMORY";
        case GENERAL: return "GENERAL";
        default: return "UNKNOWN";
    }
}