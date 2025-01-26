#ifndef RANDOMMANAGER_H
#define RANDOMMANAGER_H

#include <random>

class RandomManager {
      private:
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<int> dist;
    public:
      RandomManager();
      int get_random_int(int min, int max);
};



#endif //RANDOMMANAGER_H
