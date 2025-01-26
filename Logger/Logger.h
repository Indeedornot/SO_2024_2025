#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <chrono>
#include <string>

namespace fs = std::filesystem;

class Logger {
public:
    enum Category {
        LOCK_SEMAPHORE,
        UNLOCK_SEMAPHORE,
        PRODUCER,
        RECEIVER,
        DIRECTOR,
        SHARED_MEMORY,
        GENERAL
    };

    constexpr static int FILE_LOG = 1;
    constexpr static int CONSOLE_LOG = 2;

    Logger();
    ~Logger();

    void log(Category category, const std::string &message);
    void perror(Category category, const std::string &message);

private:
    static std::string get_category_name(Category category);

    std::unordered_set<Category> enabled_categories_;
    int log_mode_;
    std::ofstream log_file_;
};

#endif // LOGGER_H