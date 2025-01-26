#include "SleepManager.h"
#include <unistd.h>
#include "../SharedData/Globals.h"

void SleepManager::sleep_ms(const int ms) {
    if(usleep(ms * 1000) == -1) {
      logger.perror(Logger::GENERAL, "usleep() failed.");
    }
}