//
// Created by Allir on 17.01.2024.
//

#ifndef PROJEKT1_LOGGER_H
#define PROJEKT1_LOGGER_H
#include <thread>
#include <semaphore>
#include <iostream>

enum Action{
    IDLE,
    HANDLE_SIGNAL,
    DUMP,
    CHANGE_MODE,
    KILL_THREAD
};


struct StructThreadData{
    std::binary_semaphore* mainSemaphore;
    Action action;
    int signal;
    int signalData;

};

class Logger {
public:
    Logger();
    ~Logger();
private:
    std::thread *handle;
    StructThreadData threadData;

};


#endif //PROJEKT1_LOGGER_H
