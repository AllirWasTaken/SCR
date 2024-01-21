//
// Created by Allir on 17.01.2024.
//

#include "Logger.h"
#include <stdexcept>







Logger *instance=nullptr;

void SigThread(void *arg){

}


void HandlingThread(void* arg){
    auto data = (StructThreadData*)arg;



    while(true){
        data->mainSemaphore->acquire();
        if(data->action==KILL_THREAD){
            break;
        }
        data->mainSemaphore->release();
    }



}





Logger::Logger(){
    if(!instance){
        instance=this;
    }
    else{
        throw std::runtime_error("Creating instance of Logger class is not allowed");
    }
    threadData.mainSemaphore=new std::binary_semaphore(0);
    handle=new std::thread(HandlingThread,&threadData);
};

Logger::~Logger() {

    handle->join();
}