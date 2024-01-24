#include <iostream>
#include "Logger.h"


struct memory{
    int a;
};

void DumpFunction(FILE*f,void* args){
    auto arg=(memory*) args;
    fwrite(arg,sizeof(memory),1,f);
}



int main() {
    memory mem{
        .a=5
    };
    Logger::SetMemoryDumpFunction(DumpFunction,(void*)&mem);
    std::cout<<::getpid()<<std::endl;
    for(int i=0;i<1000;i++){
        sleep(1);
        Logger::NewLogEntry(std::to_string(i).c_str(),100);
    }
    return 0;
}
