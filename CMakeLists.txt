cmake_minimum_required(VERSION 3.28.3)
project(factory_v3)

set(CMAKE_CXX_STANDARD 20)

add_executable(factory_v3
        main.cpp
        Logger/Logger.cpp
        Processes/Director.cpp
        Managers/SemaphoreManager.cpp
        Managers/SleepManager.cpp
        SharedData/SharedData.cpp
        SharedData/SharedDataManager.cpp
        SharedData/Globals.cpp
        Managers/RandomManager.cpp
        Processes/Producer.cpp
        Processes/Receiver.cpp
)
