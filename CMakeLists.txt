cmake_minimum_required(VERSION 3.18.3)
project(factory_v3)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

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

target_link_libraries(factory_v3 PRIVATE Threads::Threads rt)