#include <iostream>
#include "Logger.h"

int main() {
    std::cout<<::getpid()<<std::endl;
    for(int i=0;i<1000;i++){
        sleep(1);
        Logger::NewLogEntry(std::to_string(i).c_str(),100);
    }
    return 0;
}
