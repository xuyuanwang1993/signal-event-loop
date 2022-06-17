#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/common/hid_utils.h"
#include "imp/common/iutils.h"
#include "imp/device-update/device_updater.h"
#include "imp/common/serial_utils.h"
#include<stdio.h>
using namespace aimy;
int hid_test(int argc,char *argv[]);
int update_task(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/updater","updater");
    auto print_log=getenv("AIMY_DEBUG");
    if(print_log==nullptr||strcasecmp(print_log,"false")!=0)
    {
        fprintf(stderr,"you can export AIMY_DEBUG=false to disable default log message\r\n");
        aimy::AimyLogger::Instance().set_log_to_std(true);
    }
    else {
        aimy::AimyLogger::Instance().set_log_to_std(false);
    }
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    return update_task(argc,argv);
    return  hid_test(argc,argv);
}

int hid_test(int argc,char *argv[])
{
    uint16_t product_id=std::stoul(argv[1]);
    uint16_t vendor_id=std::stoul(argv[2]);
    auto list=HidDevice::hidFind(product_id,vendor_id);
    AIMY_DEBUG("%04x:%04x",product_id,vendor_id);
    for(auto i: list)
    {
        AIMY_INFO("%s",i.c_str());
    }
    return 0;
}
void print_help_info(char *argv[])
{
    fprintf(stderr,"Usage:\r\n");
    fprintf(stderr,"\t-h|--help\tprint this page\r\n");
    fprintf(stderr,"\t-v|--version\tget the tool's version infomation\r\n");
    fprintf(stderr,"\t-d|--device [dev_path]\tquery the availiable device infomation\r\n");
    fprintf(stderr,"\t-u|--upid [dev_path]\tquery the chip infomation\r\n");
    fprintf(stderr,"\t-m|--mode [dev_path]\tquery the workmode infomation\r\n");
    fprintf(stderr,"\t--update [file_path] [device_adress] [dev_path]\tquery the workmode infomation\r\n");
    fprintf(stderr,"------------------------------\r\n");
    fprintf(stderr,"update examples:\r\n");
    fprintf(stderr,"\t%s --update test.amva 96 /dev/ttyS3\t\r\n",argv[0]);
    fprintf(stderr,"\t%s --update test.amva 96 /dev/hidraw3\t\r\n",argv[0]);
    fprintf(stderr,"------------------------------\r\n");
}

int update_task(int argc,char *argv[])
{
    if(argc<2)
    {
        print_help_info(argv);
        return -1;
    }
    std::string option=argv[1];
    if(option=="-h"||option=="--help")
    {
        print_help_info(argv);
        return 0;
    }
    else if (option=="-v"||option=="--version") {
        fprintf(stderr,"%s  %s\r\n",AIMY_UPDATER_VERSION,AIMY_UPDATER_VERSION_TIME);
        fprintf(stderr,"if there is any bug,please report the bug to %s\r\n",AIMY_UPDATER_VERSION_AUTHOR);
        return 0;
    }
    if(argc<3)
    {//need dev_path
        print_help_info(argv);
        return -1;
    }
    std::vector<std::string>param_list;
    for(int i=2;i<argc;++i)
    {
        param_list.push_back(argv[i]);
    }
    std::string dev_path=*param_list.rbegin();
    //init loop
    EventLoop loop(2);
    loop.start();
    AimyUpdater updater(loop.getTaskScheduler().get());
    if(!updater.init(dev_path))
    {
        AIMY_ERROR("init %s failed!",dev_path.c_str());
        return -2;
    }
    updater.notifyQuitMessage.connectFunc([&](UpgradeQuitStatus status,std::string reason){
        AIMY_DEBUG("recv quit status[%02x] reason[%s],exit!",status,reason.c_str());
        loop.stop();
    });
    int ret=1;
    auto timeout_timer=loop.addTimer(15000);
    timeout_timer->setSingle(true);
    timeout_timer->start();
    timeout_timer->timeout.connectFunc([&](){
        loop.stop();
        if(ret!=0)
        {
            AIMY_ERROR("exec timeout");
        }
    });
    if(option=="--update")
    {//update
        if(param_list.size()!=3)
        {
            print_help_info(argv);
            return -1;
        }
        std::string file_path=param_list[0];
        std::string dev_adress=param_list[1];
        if(!updater.loadUpdateTask(file_path,std::stoul(dev_adress)&0xffffffff))
        {
            AIMY_ERROR("load file [%s] failed!",file_path.c_str());
            return -3;
        }
        updater.notifyUpdateMessage.connectFunc([&](UpdateTypeDef cmd){
#ifdef DEBUG
            AIMY_DEBUG("recv update 0x%02x",cmd);
#else
            (void)cmd;
#endif
            timeout_timer->stop();
            timeout_timer->start();
            ret=0;
        });
        updater.notifyUpdateProgress.connectFunc([&](double progress){
            AIMY_DEBUG("update progress %lf%%",progress);
            ret=0;
        });
        updater.startUpdate();
    }
    else {
        timeout_timer->start();
        updater.notifyQueryMessage.connectFunc([&](UpdateTypeDef cmd,std::string message){
            AIMY_DEBUG("recv query[%02x] %s,exit!",cmd,message.c_str());
            timeout_timer->stop();
            timeout_timer->setInterval(2000);
            timeout_timer->start();
            ret=0;
        });
        if(option=="-d"||option=="--device")
        {
            updater.queryDevice();
        }
        else if (option=="-u"||option=="--upid") {
            updater.queryUPID();
        }
        else if (option=="-m"||option=="--mode") {
            updater.queryWorkMode();
        }
        else {
            AIMY_ERROR("unsupported option %s",option.c_str());
            loop.stop();
        }
    }
    loop.waitStop();
    AIMY_DEBUG("updater done");
    return ret;
}
