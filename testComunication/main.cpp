#include "Comunication.h"
#include <iostream>
#include <cstring>
int main(){

    /*
    mqd_t mq;
    struct mq_attr attr;
    char buffer[1024];
    attr.mq_flags=0;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=1024;
    attr.mq_curmsgs=0;

    mq= mq_open("/test_queue",O_CREAT|O_RDONLY,0644,&attr);
*/

    int d;
    printf("gib\n");
    scanf("%d",&d);
    if(d==1){
        mq_unlink("/test_queues");
        MessageQueue host;
        host.Host("/test_queues");
        messageStruct mess;
        host.GetMessage(mess);

        std::string longMess;


        for(int i=0;i<500;i++){
            longMess+=std::to_string(i)+" ";
        }




        host.SendLongMessage(longMess,"/babcia");
    }
    else if(d==3){
    }
    else{
        mq_unlink("/babcia");
        MessageQueue client;
        std::string mess;
        client.Host("/babcia");
        client.Send("/test_queues","u gay");
        client.GetLongMessage(mess);
        std::cout<<mess<<std::endl;
        std::cout<<":c";
    }




    return 0;
}