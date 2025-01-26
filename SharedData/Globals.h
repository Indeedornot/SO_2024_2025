#ifndef GLOBALS_H
#define GLOBALS_H

#include "../Logger/Logger.h"
#include "csignal"

constexpr auto SHM_NAME = "/my_shm";
constexpr auto SEM_GLOBAL_MUTEX = "/my_global_mutex";
constexpr int MAX_PRODUCERS = 3;
constexpr size_t SHM_SIZE = 1024;
constexpr int STOP_RECEIVER_SIGNAL = SIGUSR1;
constexpr int STOP_PRODUCER_SIGNAL = SIGUSR2;
constexpr int STOP_ALL_SIGNAL = SIGTERM;
constexpr int STOP_ALL_WITH_SAVE_SIGNAL = SIGINT;
extern Logger logger;

#endif // GLOBALS_H