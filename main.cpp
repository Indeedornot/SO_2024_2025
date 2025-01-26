#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <map>
#include <vector>
#include <random>
#include <memory>
#include <thread>
#include <atomic>
#include <fstream>
#include <sys/wait.h>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <optional>
#include <algorithm>

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

    Logger() {
        enabled_categories_.insert({PRODUCER, RECEIVER, DIRECTOR, SHARED_MEMORY, GENERAL});
        if (log_mode_ & FILE_LOG) {
            auto log_path = fs::path("log." + std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) + ".txt");
            log_file_.open(log_path);
            if (!log_file_) {
                std::cerr << "Failed to open log file." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    ~Logger() {
        if (log_mode_ & FILE_LOG) {
            log_file_.close();
        }
    }

    void log(const Category category, const std::string &message) {
        if (enabled_categories_.contains(category)) {
            if (log_mode_ & CONSOLE_LOG) {
                std::cout << "[" << get_category_name(category) << "] " << message << std::endl;
            }

            if (log_mode_ & FILE_LOG) {
                log_file_ << "[" << get_category_name(category) << "] " << message << std::endl;
            }
        }
    }

private:
    static std::string get_category_name(const Category category) {
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

    std::unordered_set<Category> enabled_categories_;
    int log_mode_ = FILE_LOG;
    std::ofstream log_file_;
};

constexpr auto SHM_NAME = "/my_shm";
constexpr auto SEM_PRODUCER_PREFIX = "/my_producer_sem_";
constexpr auto SEM_RECEIVER_MUTEX = "/my_receiver_mutex";
constexpr auto SEM_GLOBAL_MUTEX = "/my_global_mutex";
constexpr int MAX_PRODUCERS = 10;
constexpr size_t SHM_SIZE = 1024;
Logger logger;

struct SharedData {
    int producer_values[MAX_PRODUCERS]{};
    std::atomic<bool> stop_signal{false};
    std::atomic<bool> save_state{false};
    sem_t *global_mutex{};

    [[nodiscard]] int get_producer_value(const int producer_id) const {
        if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
            throw std::out_of_range("Invalid producer ID");
        }
        return producer_values[producer_id - 1];
    }

    void set_producer_value(const int producer_id, const int value) {
        if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
            throw std::out_of_range("Invalid producer ID");
        }
        producer_values[producer_id - 1] = value;
    }

    void increment_producer_value(const int producer_id, const int increment) {
        if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
            throw std::out_of_range("Invalid producer ID");
        }
        producer_values[producer_id - 1] += increment;
    }

    void decrement_producer_value(const int producer_id, const int decrement) {
        if (producer_id < 1 || producer_id > MAX_PRODUCERS) {
            throw std::out_of_range("Invalid producer ID");
        }
        producer_values[producer_id - 1] -= decrement;
    }
};

void clear_semaphores() {
    for (int i = 0; i <= MAX_PRODUCERS; ++i) {
        std::string sem_name = std::string(SEM_PRODUCER_PREFIX) + std::to_string(i);
        sem_unlink(sem_name.c_str());
    }
    sem_unlink(SEM_RECEIVER_MUTEX);
    sem_unlink(SEM_GLOBAL_MUTEX);
}

class SemaphoreManager {
public:
    static sem_t *create_semaphore(const std::string &name, const int value, const std::string &owner) {
        sem_t *sem = sem_open(name.c_str(), O_CREAT, 0666, value);
        if (sem == SEM_FAILED) {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
        logger.log(Logger::GENERAL, owner + " created semaphore: " + name);
        return sem;
    }

    static void lock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
        logger.log(Logger::LOCK_SEMAPHORE, owner + " attempting to lock semaphore: " + name);
        if (sem_wait(sem) == -1) {
            perror("sem_wait");
            exit(EXIT_FAILURE);
        }
        logger.log(Logger::LOCK_SEMAPHORE, owner + " locked semaphore: " + name);
    }

    static void unlock_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
        logger.log(Logger::UNLOCK_SEMAPHORE, owner + " attempting to unlock semaphore: " + name);
        if (sem_post(sem) == -1) {
            perror("sem_post");
            exit(EXIT_FAILURE);
        }
        logger.log(Logger::UNLOCK_SEMAPHORE, owner + " unlocked semaphore: " + name);
    }

    static void close_semaphore(sem_t *sem, const std::string &name, const std::string &owner) {
        sem_close(sem);
        sem_unlink(name.c_str());
        logger.log(Logger::GENERAL, owner + " closed semaphore: " + name);
    }
};

class SharedDataManager {
public:
    SharedDataManager() {
        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, SHM_SIZE);
        shared_data = static_cast<SharedData *>(mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
        new(shared_data) SharedData();
        shared_data->global_mutex = SemaphoreManager::create_semaphore(SEM_GLOBAL_MUTEX, 1, "SharedDataManager");
        std::fill(std::begin(shared_data->producer_values), std::end(shared_data->producer_values), 0);
        logger.log(Logger::SHARED_MEMORY, "Shared memory initialized.");
    }

    ~SharedDataManager() {
        shared_data->~SharedData();
        SemaphoreManager::close_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "SharedDataManager");
        munmap(shared_data, SHM_SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);
        logger.log(Logger::SHARED_MEMORY, "Shared memory cleaned up.");
    }

    [[nodiscard]] SharedData *get_shared_data() const { return shared_data; }

private:
    int shm_fd;
    SharedData *shared_data;
};

class Producer {
public:
    Producer(const int id, const int value_per_cycle, const int limit, SharedData *shared_data)
        : id(id), value_per_cycle(value_per_cycle), limit(limit), shared_data(shared_data) {
        sem_name = std::string(SEM_PRODUCER_PREFIX) + std::to_string(id);
        sem = SemaphoreManager::create_semaphore(sem_name, 1, "Producer " + std::to_string(id));
        logger.log(Logger::PRODUCER, "Producer " + std::to_string(id) + " started with limit " + std::to_string(limit));
    }

    ~Producer() {
        SemaphoreManager::close_semaphore(sem, sem_name, "Producer " + std::to_string(id));
        logger.log(Logger::PRODUCER, "Producer " + std::to_string(id) + " cleaned up.");
    }

    void run() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> time_dist(500, 2000);

        auto file_path = fs::path("producer_" + std::to_string(id) + ".txt");
        if (!fs::exists(file_path)) {
            std::ofstream file(file_path);
            file << 0 << std::endl;
        } else {
            std::ifstream file(file_path);
            int value;
            file >> value;

            SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + std::to_string(id));
            shared_data->set_producer_value(id, value);
            SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + std::to_string(id));

            logger.log(Logger::PRODUCER, "Producer " + std::to_string(id) + " loaded previous value: " + std::to_string(value));
        }

        while (!shared_data->stop_signal.load()) {
            usleep(time_dist(gen) * 1000);

            SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + std::to_string(id));

            if (shared_data->get_producer_value(id) + value_per_cycle <= limit) {
                shared_data->increment_producer_value(id, value_per_cycle);
                logger.log(Logger::PRODUCER, "Producer " + std::to_string(id) + " produced " + std::to_string(value_per_cycle) + ". Total: " + std::to_string(shared_data->get_producer_value(id)));
            } else {
                logger.log(Logger::PRODUCER, "Producer " + std::to_string(id) + " reached its limit. Skipping production.");
            }

            SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Producer " + std::to_string(id));
        }

        if (shared_data->save_state.load()) {
            std::ofstream file(file_path);
            file << shared_data->get_producer_value(id) << std::endl;
        }
    }

private:
    int id;
    int value_per_cycle;
    int limit;
    SharedData *shared_data;
    std::string sem_name;
    sem_t *sem;
};

class Receiver {
public:
    Receiver(const int id, std::map<int, int> assigned_producers, SharedData *shared_data)
        : assigned_producers(std::move(assigned_producers)), shared_data(shared_data), id(id) {
        receiver_mutex = SemaphoreManager::create_semaphore(SEM_RECEIVER_MUTEX, 1, "Receiver");
        logger.log(Logger::RECEIVER, "Receiver started.");
    }

    ~Receiver() {
        SemaphoreManager::close_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Receiver");
        logger.log(Logger::RECEIVER, "Receiver cleaned up.");
    }

    void run() {
        while (!shared_data->stop_signal.load()) {
            SemaphoreManager::lock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver");

            const bool enough_values = std::ranges::all_of(assigned_producers, [this](const auto &pair) {
                auto [producer_id, consume_amount] = pair;
                return shared_data->get_producer_value(producer_id) >= consume_amount;
            });

            if (!enough_values) {
                SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver");
                usleep(100000);
                continue;
            }

            for (const auto &[producer_id, consume_amount]: assigned_producers) {
                shared_data->decrement_producer_value(producer_id, consume_amount);
                logger.log(Logger::RECEIVER, "Receiver consumed " + std::to_string(consume_amount) + " from Producer " + std::to_string(producer_id) + ". Remaining: " + std::to_string(shared_data->get_producer_value(producer_id)));
            }

            SemaphoreManager::unlock_semaphore(shared_data->global_mutex, SEM_GLOBAL_MUTEX, "Receiver");
            usleep(500000);
        }
    }

private:
    std::map<int, int> assigned_producers;
    SharedData *shared_data;
    sem_t *receiver_mutex;
    int id;
};

class Director {
public:
    explicit Director(SharedData *shared_data) : shared_data(shared_data) {
        receiver_mutex = SemaphoreManager::create_semaphore(SEM_RECEIVER_MUTEX, 1, "Director");
        logger.log(Logger::DIRECTOR, "Director started.");
    }

    ~Director() {
        SemaphoreManager::close_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
        logger.log(Logger::DIRECTOR, "Director cleaned up.");
    }

    void run() const {
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

private:
    SharedData *shared_data;
    sem_t *receiver_mutex;

    void stop_processes(const bool save) const {
        shared_data->stop_signal.store(true);
        shared_data->save_state.store(save);
        std::cout << "Stopping all processes " << (save ? "with" : "without") << " saving." << std::endl;
    }

    void print_status() const {
        SemaphoreManager::lock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
        for (int i = 1; i <= MAX_PRODUCERS; ++i) {
            std::cout << "Producer " << i << " has total value: " << shared_data->get_producer_value(i) << std::endl;
        }
        SemaphoreManager::unlock_semaphore(receiver_mutex, SEM_RECEIVER_MUTEX, "Director");
    }
};

void create_producers(const std::map<int, std::pair<int, int>> &producers, SharedData *shared_data) {
    for (const auto &[producer_id, params]: producers) {
        if (fork() == 0) {
            Producer producer(producer_id, params.first, params.second, shared_data);
            producer.run();
            exit(0);
        }
    }
}

void create_receivers(const std::map<int, std::map<int, int>> &receiver_configs, SharedData *shared_data) {
    for (const auto &[receiver_id, assigned_producers]: receiver_configs) {
        if (fork() == 0) {
            Receiver receiver(receiver_id, assigned_producers, shared_data);
            receiver.run();
            exit(0);
        }
    }
}

int main() {
    clear_semaphores();
    const SharedDataManager shared_data_manager;
    SharedData *shared_data = shared_data_manager.get_shared_data();

    // Producer ID -> {Value per cycle, Limit}
    const std::map<int, std::pair<int, int>> producers = {
        {1, {5, 20}}, {2, {10, 30}}, {3, {3, 10}}
    };

    // Receiver ID -> {Producer ID -> Consume amount}
    const std::map<int, std::map<int, int>> receiver_configs = {
        {1, {{1, 2}, {2, 5}}},
        {2, {{2, 3}, {3, 1}}}
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