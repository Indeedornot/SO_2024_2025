#ifndef GLOBALS_H
#define GLOBALS_H

#include "../Logger/Logger.h"

constexpr auto SHM_NAME = "/my_shm";
constexpr auto SEM_PRODUCER_PREFIX = "/my_producer_sem_";
constexpr auto SEM_RECEIVER_MUTEX = "/my_receiver_mutex";
constexpr auto SEM_GLOBAL_MUTEX = "/my_global_mutex";
constexpr int MAX_PRODUCERS = 10;
constexpr size_t SHM_SIZE = 1024;
extern Logger logger;

#endif // GLOBALS_H