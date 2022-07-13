#include "imp/commandlineTool.h"
#include "imp/localCommandlineTool.h"
#include "log/aimy-log.h"
#include <unistd.h>
#include <chrono>
int main(int argc, char *argv[])
{
    aimy::AimyLogger::Instance().set_log_to_std(true);
    aimy::AimyLogger::Instance().set_log_path("./logs","commandLineTest");
    aimy::AimyLogger::Instance().register_handle();
    aimy::commandLineTestTool tool;
    tool.handleCommandlineCmd(argc,argv);
    std::thread t([&](){
        while(1)
        {
            tool.multicastMessage("connection check message");
            sleep(5);
        }
    });
    t.detach();
    tool.start();
    tool.waitDone();
    aimy::AimyLogger::Instance().unregister_handle();
    return 0;
}
