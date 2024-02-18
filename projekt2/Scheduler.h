#ifndef PROJEKT2_SCHEDULER_H
#define PROJEKT2_SCHEDULER_H
#include "Comunication.h"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <memory.h>
#include <cstdlib>
#include <unistd.h>
#include "Logger.h"
#define MAX_COMMAND_SIZE 300


struct scheduledTask{
    int years;
    int days;
    int hours;
    int minutes;
    int seconds;
    int mode;
    char command[MAX_COMMAND_SIZE];
};

class timerTask{
public:
    unsigned long long timeWhen;
    int mode;
    unsigned long long id;
    timer_t timer;
    sem_t *critical;
    std::vector<timerTask*>* taskVector;
    char command[MAX_COMMAND_SIZE];
    static void DeleteTimer(timerTask* task);
};


class Scheduler {
private:
    MessageQueue host;
    sem_t criticalSemaphore;
    std::vector<timerTask*> taskVector;
    std::vector<pthread_t> taskHandles;
    bool work=true;

public:
    Scheduler();
    ~Scheduler();
    void HostScheduler();
    void CancelTask(unsigned long long id);
    friend std::ostream &operator<<(std::ostream &stream,Scheduler& a);


};


#endif //PROJEKT2_SCHEDULER_H
