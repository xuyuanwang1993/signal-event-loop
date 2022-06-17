#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/bluez-looptest/at-device-task.h"
#include "imp/bluez-looptest/bluez-test-task.h"
using namespace aimy;
static int bluez_looptest(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    return bluez_looptest(argc,argv);
}

int bluez_looptest(int argc,char *argv[])
{
    std::string device_name;
    if(argc>1)device_name=argv[1];
    EventLoop loop;
    loop.start();
    std::shared_ptr<AtDeviceTask>task( new AtDeviceTask(loop.getTaskScheduler().get()));
    std::shared_ptr<BluetoothTest>testTask(new BluetoothTest(loop.getTaskScheduler().get()));
    task->notifyMac.connect(testTask.get(),std::bind(&BluetoothTest::on_recv_mac,testTask.get(),std::placeholders::_1,std::placeholders::_2));
    testTask->finshTest.connectFunc([&](int ret){
        AIMY_WARNNING("finsh test ret[%d]",ret);
        _Exit(ret);
        std::thread t_exit([&](){
            AIMY_WARNNING("stop------1-----------");
            task->stop();
            AIMY_WARNNING("stop------2-----------");
            testTask->stop();
            AIMY_WARNNING("stop------3-----------");
            loop.stop();
            AIMY_WARNNING("stop------finish-----------");
        });
        t_exit.detach();
    });
    testTask->start();
    if(device_name.empty())task->start();
    else {
        task->start(device_name);
    }


    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        AIMY_WARNNING("stop------11-----------");
        task->stop();
        AIMY_WARNNING("stop------22-----------");
        testTask->stop();
        AIMY_WARNNING("stop------33-----------");
        loop.stop();
        AIMY_WARNNING("stop------finish-----------");
    });
    loop.waitStop();
    return testTask->result();;
}
