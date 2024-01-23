
#include "Logger.h"
#include <stdexcept>


Logger *instance=nullptr;
Logger loggerObject;

#define SIG_HANDLER(SIG) \
void handler_##SIG(int signo, siginfo_t* info, void* other)\
{\
    instance->signal_##SIG.signalData.store(info->si_value.sival_int);\
    instance->signal_##SIG.signalSemaphore->release();     \
}

SIG_HANDLER(SIGUSR1)//DUMP
SIG_HANDLER(SIGUSR2)//ON OFF
SIG_HANDLER(SIGCONT)//CHANGE MODE


void HandlingThread_SIGUSR1(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGUSR1);
    pthread_sigmask(SIG_SETMASK,&set,nullptr);


    while(true){
        instance->signal_SIGUSR1.signalSemaphore->acquire();

        instance->criticalSemaphore->acquire();
        if(!instance->work){
            instance->criticalSemaphore->release();
            return;
        }


        //DUMP

        instance->criticalSemaphore->release();
    }

}

void HandlingThread_SIGUSR2(){

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGUSR2);
    pthread_sigmask(SIG_SETMASK,&set,nullptr);

    while(true){
        instance->signal_SIGUSR2.signalSemaphore->acquire();

        instance->criticalSemaphore->acquire();
        if(!instance->work){
            instance->criticalSemaphore->release();
            return;
        }


        instance->logging=!instance->logging;

        instance->criticalSemaphore->release();
    }

}

void HandlingThread_SIGCONT(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGCONT);
    pthread_sigmask(SIG_SETMASK,&set,nullptr);


    while(true){
        instance->signal_SIGCONT.signalSemaphore->acquire();

        instance->criticalSemaphore->acquire();
        if(!instance->work){
            instance->criticalSemaphore->release();
            return;
        }


        if(instance->signal_SIGCONT.signalData.load()==0){
            instance->mode=LOG_MAX;
        }
        else if(instance->signal_SIGCONT.signalData.load()==1){
            instance->mode=LOG_STD;
        }
        else if(instance->signal_SIGCONT.signalData.load()==2){
            instance->mode=LOG_MIN;
        }

        instance->criticalSemaphore->release();
    }

}

void Logger::NewLogEntry(const char *message, int priority) {
    instance->criticalSemaphore->acquire();
    if(!instance->work||!instance->logging){
        instance->criticalSemaphore->release();
        return;
    }

    if(priority<=instance->mode){

        auto now = instance->GetCurrentDateAndTime();
        std::string logData = now +"    "+std::to_string(priority)+"                "+message+"\n";
        fprintf(instance->loggingFile,"%s",logData.c_str());

    }

    instance->criticalSemaphore->release();
}


std::string Logger::GetCurrentDateAndTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d %m %Y %H-%M-%S");
    auto str = oss.str();
    return str;
}

Logger::Logger(){
    if(!instance){
        instance=this;
    }
    else{
        throw std::runtime_error("Creating instance of Logger class is not allowed");
    }


    //init
    work=true;
    auto now=GetCurrentDateAndTime();
    std::string fileName="Log "+now;
    loggingFile=fopen(fileName.c_str(),"w");
    if(!loggingFile){
        throw std::runtime_error("Cannot create a logging file");
    }
    setvbuf( loggingFile, nullptr, _IONBF, 0 );
    fprintf(loggingFile,"Time                   priority         log information\n");

    logging=true;
    mode=LOG_STD;
    struct sigaction act;
    sigset_t set;
    sigfillset(&set);
    criticalSemaphore=new std::binary_semaphore(1);

    //sigusr1 thread
    act.sa_sigaction = handler_SIGUSR1;
    act.sa_mask = set;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1,&act,nullptr);
    signal_SIGUSR1.signalSemaphore=new std::counting_semaphore<100>(0);
    handle_USR1=new std::thread(HandlingThread_SIGUSR1);

    //sigusr2 thread
    act.sa_sigaction = handler_SIGUSR2;
    act.sa_mask = set;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR2,&act,nullptr);
    signal_SIGUSR2.signalSemaphore=new std::counting_semaphore<100>(0);
    handle_USR2=new std::thread(HandlingThread_SIGUSR2);
    //sigcont thread
    act.sa_sigaction = handler_SIGCONT;
    act.sa_mask = set;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGCONT,&act,nullptr);
    signal_SIGCONT.signalSemaphore=new std::counting_semaphore<100>(0);
    handle_CONT=new std::thread(HandlingThread_SIGCONT);

    //set masks
    sigemptyset(&set);
    sigaddset(&set,SIGUSR1);
    sigaddset(&set,SIGUSR2);
    sigaddset(&set,SIGCONT);
    pthread_sigmask(SIG_SETMASK,&set,NULL);
};

Logger::~Logger() {

    criticalSemaphore->acquire();
    work=false;
    criticalSemaphore->release();

    signal_SIGUSR1.signalSemaphore->release();
    signal_SIGUSR2.signalSemaphore->release();
    signal_SIGCONT.signalSemaphore->release();

    handle_USR1->join();
    handle_USR2->join();
    handle_CONT->join();

    fprintf(loggingFile,"Logging library is closing safely");
    fclose(loggingFile);

    delete handle_USR1;
    delete handle_USR2;
    delete handle_CONT;

    delete signal_SIGUSR1.signalSemaphore;
    delete signal_SIGUSR2.signalSemaphore;
    delete signal_SIGCONT.signalSemaphore;

    delete criticalSemaphore;
}
