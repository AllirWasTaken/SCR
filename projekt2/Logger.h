#ifndef PROJEKT1_LOGGER_H
#define PROJEKT1_LOGGER_H
#include <thread>
#include <semaphore>
#include <iostream>
#include <vector>
#include <atomic>
#include <csignal>
#include <ctime>
#include <iomanip>

#define LOG_MAX 255
#define LOG_STD 125
#define LOG_MIN 10

struct SigStruct{
    std::atomic<int> signalData;
    std::counting_semaphore<100>* signalSemaphore;
};

class Logger {
public:
    Logger();
    ~Logger();
    SigStruct signal_SIGUSR1;//DUMP
    SigStruct signal_SIGUSR2;//ON OFF
    SigStruct signal_SIGCONT;//CHANGE MODE
    std::binary_semaphore* criticalSemaphore;
    bool logging;
    int mode;
    bool work;
    void (*dumpFunction)(FILE*,void*)=nullptr;
    FILE* loggingFile;
    void* dumpArgs;

    //public usage functions
    static void NewLogEntry(const char* message,int priority);
    static void SetMemoryDumpFunction( void (*func)(FILE*,void*),void* args);
    std::string GetCurrentDateAndTime();

private:
    std::thread *handle_USR1,*handle_USR2,*handle_CONT;



};


#endif //PROJEKT1_LOGGER_H
