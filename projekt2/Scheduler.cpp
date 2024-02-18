#include <csignal>
#include "Scheduler.h"

#include "time.h"
#define TM_YEAR_BASE 1900

/* Return 1 if YEAR + TM_YEAR_BASE is a leap year.  */
static inline int
leapyear (int year)
{
    /* Don't add YEAR to TM_YEAR_BASE, as that might overflow.
       Also, work even if YEAR is negative.  */
    return
            ((year & 3) == 0
             && (year % 100 != 0
                 || ((year / 100) & 3) == (- (TM_YEAR_BASE / 100) & 3)));
}

#define MONTHS_IN_YEAR 12
const unsigned short int __mon_yday[2][13] =
        {
                /* Normal years.  */
                { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
                /* Leap years.  */
                { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
        };
class MyDateClass {
public:
    MyDateClass(int year, int dayOfYear) {
        int yearOffset = dayOfYear - TM_YEAR_BASE;
        int leapYearIndex = leapyear(year) ? 1 : 0;
        int daysInYear = leapYearIndex ? 366 : 365;

        this->year = year;
        this->dayOfYear = dayOfYear;

        if (dayOfYear >= 1 && dayOfYear <= daysInYear) {
            for (int mon = 0; mon < MONTHS_IN_YEAR; mon++) {
                if (dayOfYear <= __mon_yday[leapYearIndex][mon+1]) {
                    month = mon + 1;
                    dayOfMonth = dayOfYear - __mon_yday[leapYearIndex][mon];
                    break;
                }
            }
        } else {
            month = 0;
            dayOfMonth = 0;
        }
    }

// Get month 1=January, 12=December
    inline int getMonth() { return month; }

// Get day of month
    inline int getDayOfMonth() { return dayOfMonth; }

// Get year
    inline int getYear() { return year; }

// Get day of yar
    inline int getDayOfYear() { return dayOfYear; }

private:
    int month;
    int dayOfMonth;
    int year;
    int dayOfYear;
};

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}



void* timerFunction(void *arg){

    auto timerArgs=(timerTask*)arg;
    auto critical=timerArgs->critical;
    sem_wait(critical);


    char program[200];
    int counter;
    int i;


    char programName[]="program";

    std::vector<std::string> splited;
    counter=(int)split(timerArgs->command,splited,' ');
    char** arr=new char*[counter+1];

    strcpy(program,splited[0].c_str());
    arr[0]=programName;
    for(i=1;i<counter;i++){
        arr[i]=(char*)splited[i].c_str();
    }
    arr[counter]= nullptr;



    if(!fork()){
        execv(program,arr);
    }



    delete[] arr;

     /*
    std::string launch="konsole --separate -e ";
    launch+=timerArgs->command;
    Logger::NewLogEntry(launch.c_str(),0);
    system(launch.c_str());
      */
    if(timerArgs->mode==0||timerArgs->mode==1){
        timerTask::DeleteTimer(timerArgs);
    }
    else{
        itimerspec value;
        value.it_value.tv_sec=(long)timerArgs->timeWhen;
        value.it_value.tv_nsec=0;
        value.it_interval.tv_sec=0;
        value.it_interval.tv_nsec=0;

        timer_settime((*timerArgs->taskVector)[timerArgs->taskVector->size()-1]->timer,0,&value,nullptr);
    }
    sem_post(critical);

    return nullptr;
}

void Scheduler::HostScheduler() {
    long long unsigned idCounter=0;

    scheduledTask newTask;

    while(true) {

        messageStruct mess;
        host.GetMessage(mess);
        memcpy(&newTask, mess.data, sizeof(scheduledTask));

        if(newTask.mode==3){


            std::stringstream taskStream;

            taskStream<<*this;
            std::string taskList;
            taskList=taskStream.str();
            host.SendLongMessage(taskList,mess.queueSender);
            continue;
        }
        else if(newTask.mode==4){
            break;
        }
        else if(newTask.mode==5){
            CancelTask(newTask.years);
            continue;
        }

        timerTask *newTaskPtr=new timerTask;
        timerTask &newTimerTask=*newTaskPtr;
        newTimerTask.critical = &criticalSemaphore;
        newTimerTask.taskVector = &taskVector;
        newTimerTask.mode = newTask.mode;
        newTimerTask.id = idCounter;
        idCounter++;
        strcpy(newTimerTask.command, newTask.command);
        if (newTask.mode == 0 || newTask.mode == 2) {
            newTimerTask.timeWhen = 0;
            newTimerTask.timeWhen += newTask.years * 365 + newTask.days;
            newTimerTask.timeWhen *= 24;
            newTimerTask.timeWhen += newTask.hours;
            newTimerTask.timeWhen *= 60;
            newTimerTask.timeWhen += newTask.minutes;
            newTimerTask.timeWhen *= 60;
            newTimerTask.timeWhen += newTask.seconds;
        } else {
            MyDateClass date(newTask.years, newTask.days);
            tm t;
            t.tm_year = newTask.years;
            t.tm_mon = date.getMonth();
            t.tm_mday = date.getDayOfMonth();
            t.tm_hour = newTask.hours;
            t.tm_min = newTask.minutes;
            t.tm_sec = newTask.seconds;

            auto nwo = time(nullptr);
            struct tm tm = *localtime(&nwo);
            auto val2 = mktime(&tm);

            auto val = mktime(&t);

            long seconds = val - val2;
            newTimerTask.timeWhen = seconds;
        }


        sem_wait(&criticalSemaphore);
        taskVector.push_back(newTaskPtr);
        sigevent timerEvent;
        timerEvent.sigev_notify = SIGEV_THREAD;
        timerEvent.sigev_notify_function = reinterpret_cast<void (*)(__sigval_t)>(timerFunction);
        timerEvent.sigev_value.sival_ptr = taskVector[taskVector.size() - 1];
        timerEvent.sigev_notify_attributes = nullptr;
        timer_create(CLOCK_REALTIME, &timerEvent, &(taskVector[taskVector.size() - 1]->timer));

        itimerspec value;
        value.it_value.tv_sec = (long) newTimerTask.timeWhen;
        value.it_value.tv_nsec = 0;
        value.it_interval.tv_sec = 0;
        value.it_interval.tv_nsec = 0;

        timer_settime(taskVector[taskVector.size() - 1]->timer, 0, &value, nullptr);
        Logger::NewLogEntry("Added new task to the task list", 100);

        sem_post(&criticalSemaphore);
    }
}


Scheduler::Scheduler() {
    host.Host("/Scheduler");
    sem_init(&criticalSemaphore,0,1);
}

Scheduler::~Scheduler(){
    work=false;
    host.Close();
    sem_wait(&criticalSemaphore);
    for(int i=0;i<taskVector.size();i++){
        timerTask::DeleteTimer(taskVector[i]);
    }

    sem_destroy(&criticalSemaphore);

}

std::ostream& operator<<(std::ostream &stream,Scheduler& a){
    sem_wait(&a.criticalSemaphore);
    for(int i=0;i<a.taskVector.size();i++){
        stream<<"\nTask "<<a.taskVector[i]->id
        <<"\nTimeSet (seconds): "<<a.taskVector[i]->timeWhen
        <<"\nMode:";
        if(a.taskVector[i]->mode==0)stream<<"Once";
        if(a.taskVector[i]->mode==1)stream<<"Once";
        if(a.taskVector[i]->mode==2)stream<<"Repeat";
        stream<<"\nCommand: "<<a.taskVector[i]->command<<std::endl;
    }
    sem_post(&a.criticalSemaphore);
    return stream;
}

void Scheduler::CancelTask(unsigned long long id) {
    sem_wait(&criticalSemaphore);

    for(int i=0;i<taskVector.size();i++){
        if(taskVector[i]->id==id){
            timerTask::DeleteTimer(taskVector[i]);
            break;
        }
    }

    sem_post(&criticalSemaphore);
}

void timerTask::DeleteTimer(timerTask *task) {
    auto vec_tasks=task->taskVector;
    for(int i=0;i<vec_tasks->size();i++){
        if((*vec_tasks)[i]==task){
            timer_delete((*vec_tasks)[i]->timer);
            vec_tasks->erase(vec_tasks->begin()+i);
        }
    }
}