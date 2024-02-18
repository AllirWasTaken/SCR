#include <unistd.h>
#include "Scheduler.h"
#include <cstring>




/*
  program przyjmuje argumenty, nastomiast jesli ich nie dostanie, brane są pod uwage wartosci default
  procz komendy która zawsze musi być przekazana
  wywołujemy komende a nastepnie po spacji jej argument
  -y    rok (0-inf)
  -d    dni (0-364) default=0
  -h    godziny (0-23) default=0
  -m    minuty (0-59) default=0
  -s    sekundy (0-59) default=0
  -mode    cyklicznosc (0=once,1=relative,2=repeat)
  -command komenda (text) !!Powinna być zawsze podana jako ostatnia
  -tasks lista zadan
  -shutdown wylacz
  -cancel (int) anuluj zadanie o danym id

 */

void HandleArguments(int argc, char** argv){
    bool recv=false;
    scheduledTask newTask{
            .years=0,
            .days=0,
            .hours=0,
            .minutes=0,
            .seconds=0,
            .mode=0
    };
    newTask.command[0]='\0';

    if(argc==1){
        throw std::invalid_argument("Not enough arguments provided");
    }
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"-y")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.years=value;
            i++;
            continue;
        }
        else if(!strcmp(argv[i],"-d")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0||value>364){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.days=value;
            i++;
            continue;
        }
        else if(!strcmp(argv[i],"-h")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0||value>23){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.hours=value;
            i++;
            continue;
        }
        else if(!strcmp(argv[i],"-m")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0||value>60){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.minutes=value;
            i++;
            continue;
        }
        else if(!strcmp(argv[i],"-s")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0||value>60){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.seconds=value;
            i++;
            continue;
        } else if(!strcmp(argv[i],"-mode")){
            if(newTask.mode>2)continue;
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value!=0&&value!=1&&value!=2){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.mode=value;
            i++;
            continue;
        }
        else if(!strcmp(argv[i],"-command")){
            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            strcpy(newTask.command,argv[i+1]);
            i+=2;
            for(;i<argc;i++){
                strcat(newTask.command," ");
                strcat(newTask.command,argv[i]);
            }
            continue;
        }
        else if(!strcmp(argv[i],"-tasks")){
            recv=true;
            newTask.mode=3;
            break;
        }
        else if(!strcmp(argv[i],"-shutdown")){
            newTask.mode=4;
            break;
        }
        else if(!strcmp(argv[i],"-cancel")){

            if(i+1==argc){
                throw std::invalid_argument("Wrong arguments provided");
            }
            int value = atoi(argv[i+1]);
            if(value<0){
                throw std::invalid_argument("Wrong arguments provided");
            }
            newTask.mode=5;
            newTask.years=value;
            break;
        }
        else{
            throw std::invalid_argument("Argument not found");
        }
    }

    if(newTask.command[0]=='\0'){
        if(newTask.mode<3)throw std::invalid_argument("Command not found");
    }

    MessageQueue client;
    char buffer[500]={0};
    memcpy(buffer,&newTask,sizeof(scheduledTask));
    if(recv){
        srand(time(nullptr));
        std::string name;
        while(true){
            name="/"+std::to_string(rand());
            if(!MessageQueue::DoesHostExist(name.c_str())){
                break;
            }
        }
        client.Host(name.c_str());
        client.Send("/Scheduler",buffer,500);

        std::string recvList;
        client.GetLongMessage(recvList);
        std::cout<<recvList<<std::endl;
    }
    else client.Send("/Scheduler",buffer,500);

}
int main(int argc, char** argv) {
    //mq_unlink("/Scheduler");
    //return 1;
    if(MessageQueue::DoesHostExist("/Scheduler")){
        HandleArguments(argc,argv);
        return 0;
    }
    Scheduler scheduler;
    try{
        if(argc>1){
            HandleArguments(argc,argv);
        }
    }
    catch (std::invalid_argument &e){
        std::cout<<"Wrong argument list provided, server will continue to work but arguments are ignored\n"<<
        "error message: "<<e.what()<<std::endl;
    }

    scheduler.HostScheduler();

    return 0;
}
