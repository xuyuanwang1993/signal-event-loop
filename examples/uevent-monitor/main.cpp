#include "log/aimy-log.h"
#include "imp/uevent_monitor.h"
using namespace aimy;
int main(int argc,char *argv[])
{
    AimyLogger::setThreadName("main");
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    //aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
    aimy::AimyLogger::Instance().set_log_to_std(true);

    EventLoop loop(1);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        aimy::AimyLogger::Instance().unregister_handle();
        loop.stop();
    });
    loop.start();
    UeventMonitor monitor(loop.getTaskScheduler().get());
    monitor.notifyUevent.connectFunc([](std::shared_ptr<Iuevent> event){
        AIMY_DEBUG("\n\n----------------uevent start-------------------------");
        AIMY_WARNNING("action:%s",event->action);
        AIMY_WARNNING("devpath:%s",event->devpath);
        AIMY_INFO("############env##############");
        for(int i=0;i<32&&event->envp[i]!=nullptr;++i)
        {
            AIMY_WARNNING("%s",event->envp[i]);
        }
        AIMY_DEBUG("\n\n-------------------uevent end----------------------");
    });
    monitor.start();
    loop.waitStop();
    return 0;
}
