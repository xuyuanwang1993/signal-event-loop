#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/device-discover/device-discover.h"
using namespace aimy;
int discover_test(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    //return object_test(argc,argv);
    return discover_test(argc,argv);
}
int discover_test(int argc,char *argv[])
{
    if(argc<2){
        printf("Usage:%s server/client\r\n",argv[0]);
        printf("export IP_PREFIX=\"10.0.4\" to discover in 10.0.4.1~10.0.4.254\r\n");
        return -1;
    }
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto t=loop.getTaskScheduler();
    std::string option=argv[1];
    if(option=="server")
    {
        DeviceDiscover server(t.get(),DEFAULT_AGENT,true);
        server.start(loop.getThreadPool());
        server.setUpdateTimerState(true);
        loop.waitStop();
    }
    else if(option=="client"){
        DeviceDiscover server(t.get(),DEFAULT_AGENT,false);
        server.start(nullptr);
        server.notifyMessage.connectFunc([&](std::string msg){
            AIMY_DEBUG("recv\n%s",msg.c_str());
            loop.getThreadPool()->enqueue([](){
                sleep(10);
                exit(0);
            });
        });
        std::string match_str="";
        if(argc>2)
        {
            match_str=argv[2];
        }
        auto ip_prefix=getenv("IP_PREFIX");
        std::string ip_prefix_str="";
        if(ip_prefix)ip_prefix_str=ip_prefix;
        if(!ip_prefix_str.empty())
        {
            for(int i=1;i<=254;++i)
            {
                std::string ip=ip_prefix_str+"."+std::to_string(i);
                server.sendDiscover(match_str,ip);
                usleep(10000);
            }
        }
        else {
            server.sendDiscover(match_str);
        }
        loop.waitStop();
    }
    else {
        return -1;
    }
    return 0;
}
