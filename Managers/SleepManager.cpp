#include "SleepManager.h"
#include <unistd.h>

void SleepManager::sleep_ms(const int ms) {
    usleep(ms * 1000);
}