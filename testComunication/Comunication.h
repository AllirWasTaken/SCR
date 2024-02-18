#ifndef PROJEKT2_COMUNICATION_H
#define PROJEKT2_COMUNICATION_H


#include <semaphore.h>
#include <string>
#include <mqueue.h>
#define BUFFER_SIZE 1024
#define HEADER_SIZE 256
#define SAFETY_CUT 16
#define MAX_MESSAGE_COUNT 10

struct messageStruct{
    char queueSender[HEADER_SIZE];
    char data[BUFFER_SIZE-HEADER_SIZE];
};

class MessageQueue{
private:
    mqd_t ownedQueue;
    char queueName[256];
    struct mq_attr attr;
public:
    MessageQueue();
    ~MessageQueue();

    void Host(const char * name);
    void GetMessage(messageStruct &message);
    void SendLongMessage(std::string &message, const char* destName);
    void GetLongMessage(std::string &message);
    void Close();
    void Send(const char * queueName,const char* message,int len=-1);
    static bool DoesHostExist(const char * name);
};


#endif //PROJEKT2_COMUNICATION_H
