#include <stdexcept>
#include <cstring>
#include "Comunication.h"

MessageQueue::MessageQueue() {
    ownedQueue=-1;
}

void MessageQueue::Close() {
    if(ownedQueue!=-1){
        mq_close(ownedQueue);
        ownedQueue=-1;
        mq_unlink(queueName);
    }
}

MessageQueue::~MessageQueue() {
    Close();
}

void MessageQueue::Host(const char *name) {

    attr.mq_maxmsg=MAX_MESSAGE_COUNT;
    attr.mq_msgsize=BUFFER_SIZE;
    attr.mq_flags=0;
    attr.mq_curmsgs=0;
    ownedQueue=mq_open(name,O_CREAT|O_RDONLY|O_EXCL,0664,&attr);
    if(ownedQueue==-1){
        throw std::runtime_error("Failed to create queue");
    }
    strcpy(queueName,name);
}


void MessageQueue::GetMessage(messageStruct &message) {
    if(ownedQueue==-1){
        throw std::runtime_error("Queue is not hosted");
    }
    mq_receive(ownedQueue,(char*)&message,BUFFER_SIZE,nullptr);
}

void MessageQueue::Send(const char *destName,const char *message,int len) {
    int queue=mq_open(destName,O_WRONLY);
    if(queue==-1){
        throw std::runtime_error("Failed to open existing queue");
    }
    messageStruct newMessage;
    memset(newMessage.data,0,BUFFER_SIZE-HEADER_SIZE);
    if(len==-1) {
        strcpy(newMessage.data, message);
    }
    else{
        memcpy(newMessage.data, message,len);
    }
    if(ownedQueue!=-1){
        strcpy(newMessage.queueSender,queueName);
    }
    else{
        memset(newMessage.queueSender,0,HEADER_SIZE);
    }
    mq_send(queue,(char*)&newMessage,BUFFER_SIZE,1);
    //mq_notify();
    mq_close(queue);
}

bool MessageQueue::DoesHostExist(const char* name) {
    int queue=mq_open(name,O_WRONLY);
    if(queue==-1)return false;
    return true;
}

void MessageQueue::SendLongMessage(std::string &message,const char* destName) {
    int packSize=BUFFER_SIZE-HEADER_SIZE-SAFETY_CUT;

    int leftToSend=message.size();
    char packData[BUFFER_SIZE-HEADER_SIZE-SAFETY_CUT];

    int currentPos=0;

    while(true){
        if(leftToSend<packSize){
            break;
        }
        memcpy(packData,message.data()+currentPos,packSize);
        Send(destName,packData,packSize);
        currentPos+=packSize;
        leftToSend-=packSize;
    }
    if(leftToSend>0) {
        memset(packData,0,packSize);
        memcpy(packData, message.data() + currentPos, leftToSend);
        Send(destName, packData, leftToSend);
    }

    packData[0]=1;
    packData[1]=2;
    packData[2]=3;
    packData[3]=4;
    packData[4]=5;
    packData[5]=0;

    Send(destName,packData,6);
}

void MessageQueue::GetLongMessage(std::string &message) {
    int packSize=BUFFER_SIZE-HEADER_SIZE-SAFETY_CUT;
    char packData[BUFFER_SIZE-HEADER_SIZE]={0};

    messageStruct mess;

    message.clear();

    while(true){
        memset(mess.data,0,packSize);
        GetMessage(mess);
        memcpy(packData,mess.data,750);

        if(packData[0]==1){
            int check=1;
            if(packData[1]==2)check++;
            if(packData[2]==3)check++;
            if(packData[3]==4)check++;
            if(packData[4]==5)check++;
            if(packData[5]==0)check++;
            if(check==6)break;
        }

        message+=packData;

    }
}