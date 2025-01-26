#ifndef SHAREDDATAMANAGER_H
#define SHAREDDATAMANAGER_H

#include <iostream>
#include "./SharedData.h"

class SharedDataManager {
public:
    SharedDataManager();
    ~SharedDataManager();
    [[nodiscard]] SharedData *get_shared_data() const;

private:
    int shm_fd;
    SharedData *shared_data;
};

#endif //SHAREDDATAMANAGER_H
