#include <iostream>
#include "daemon-instance.h"
#include<random>
using namespace aimy;

void session_test(char *argv[])
{

    DaemonSession session(nullptr);
    session.loadConfig(argv[1]);
    std::random_device rd;
    int chance_cnt=10;
    while(1)
    {
        session.check();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while(chance_cnt--<=0)
        {
            int operation=rd()%3;
            AIMY_WARNNING("operartion %d",operation);
            if(operation==0)session.start();
            else if (operation==1) {
                session.restart();
            }
            else {
                session.stop();
            }
            chance_cnt=10;
        }
        auto ret=session.getStatusString();
    }
}
void worker_test(char *argv[])
{
     DaemonWorker WOER(argv[1]);
     WOER.start();
    std::random_device rd;
    int chance_cnt=10;
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while(chance_cnt--<=0)
        {
            int operation=rd()%4;
            AIMY_WARNNING("operartion %d",operation);
            if(operation==0)WOER.startTask("app");
            else if (operation==1) {
                WOER.restartTask("app");
            }
            else if (operation==2) {
                WOER.reloadTask("app");
            }
            else {
                WOER.cancelTask("app");

            }
            AIMY_WARNNING("operartion %d end",operation);
            chance_cnt=10;
        }
    }
}

int log_test()
{
    auto logger1=AimyLogger::create();
    {
        logger1->set_log_path("./logs","test1");
        logger1->set_max_log_file_cnts(3);
        logger1->register_handle(false);
    }
    auto logger2=AimyLogger::create();
    {

        logger2->set_log_path("./logs","test2");
        logger2->set_max_log_file_cnts(3);
        logger2->register_handle(false);
    }
    auto logger3=AimyLogger::create();
    {
        logger3->set_log_path("./logs","test3");
        logger3->set_max_log_file_cnts(1);
        logger3->register_handle(true);

    }
    AimyLogger::Instance().set_log_path("./logs","main");
    AimyLogger::Instance().set_max_log_file_cnts(3);
    AimyLogger::Instance().register_handle(true);
    int cnt=0;
    while(1)
    {
        switch (cnt%3) {
        case 0:
            AIMY_LOG(logger1,aimy::LOG_DEBUG,"log1:%d",cnt);
            AIMY_DEBUG("log1:%d",cnt);
            break;
        case 1:
            AIMY_LOG(logger2,aimy::LOG_DEBUG,"log2:%d",cnt);
            AIMY_DEBUG("log2:%d",cnt);
            break;
        case 2:
            AIMY_LOG(logger3,aimy::LOG_DEBUG,"log3:%d",cnt);
            AIMY_DEBUG("log3:%d",cnt);
            break;
        default:
            break;
        };
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ++cnt;
    }
    return 0;
}
int main(int argc ,char *argv[])
{
    //return log_test();
    Daemon::handleCommandline(argc,argv);
    return 0;
}
