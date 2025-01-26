cmake_minimum_required(VERSION 3.30)
project(factory_v3)

set(CMAKE_CXX_STANDARD 20)

add_executable(factory_v3
        main.cpp
        Logger/Logger.cpp
        Processes/Producer.cpp
        Processes/Receiver.cpp
        Processes/Director.cpp
        Managers/SemaphoreManager.cpp
        Managers/SleepManager.cpp
        SharedData/SharedData.cpp
        SharedData/SharedDataManager.cpp
        SharedData/Globals.cpp
)