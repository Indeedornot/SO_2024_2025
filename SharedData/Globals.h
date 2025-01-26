#ifndef GLOBALS_H
#define GLOBALS_H

#include "../Logger/Logger.h"
#include "csignal"

constexpr auto SHM_NAME = "/my_shm";
constexpr auto SEM_PRODUCER_PREFIX = "/my_producer_sem_";
constexpr auto SEM_RECEIVER_MUTEX = "/my_receiver_mutex";
constexpr auto SEM_GLOBAL_MUTEX = "/my_global_mutex";
constexpr int MAX_PRODUCERS = 10;
constexpr size_t SHM_SIZE = 1024;
constexpr int STOP_RECEIVER_SIGNAL = SIGUSR1;
constexpr int STOP_PRODUCER_SIGNAL = SIGUSR2;
constexpr int STOP_ALL_SIGNAL = SIGTERM;
constexpr int STOP_ALL_WITH_SAVE_SIGNAL = SIGKILL;
extern Logger logger;

#endif // GLOBALS_H