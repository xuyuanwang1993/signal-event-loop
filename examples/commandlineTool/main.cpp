#include "imp/commandlineTool.h"
#include "log/aimy-log.h"
int main(int argc, char *argv[])
{
    aimy::AimyLogger::Instance().set_log_to_std(true);
    aimy::AimyLogger::Instance().set_log_path("./logs","commandLineTest");
    aimy::AimyLogger::Instance().register_handle();
    aimy::commandLineTestTool tool;
    tool.handleCommandlineCmd(argc,argv);
    tool.start();
    tool.waitDone();
    aimy::AimyLogger::Instance().unregister_handle();
    return 0;
}
