#include "at-device-task.h"
#include "imp/common/serial_utils.h"
#include<strings.h>
#include<stdlib.h>
#include<dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
using namespace aimy;

AtDeviceTask::AtDeviceTask(Object *parent):Object(parent),notifyMac(this),matchName(""),matchedName(""),findMacTimer(nullptr)
  ,workThread(nullptr),threadRunning(false)
{
    findMacTimer=this->addTimer(1000);
    findMacTimer->setSingle(true);
    findMacTimer->timeout.connect(this,std::bind(&AtDeviceTask::on_find_mac_time_out,this));
}

AtDeviceTask::~AtDeviceTask()
{
    DefaultObjectHandler::instance().addObjectEvent([this](){
        stop();
        if(findMacTimer)findMacTimer->release();
    });
}

void AtDeviceTask::start(const std::string &ttyDeviceMatchName)
{
    AIMY_DEBUG("start AT device task %s",ttyDeviceMatchName.c_str());
    invoke(Object::getCurrentThreadId(),[=](){
        if(threadRunning)
        {
            AIMY_WARNNING("AT task thread is working!");
            return ;
        }
        matchName=ttyDeviceMatchName;
        on_find_mac_time_out();
    });
}

void AtDeviceTask::stop()
{
    invoke(Object::getCurrentThreadId(),[=](){
        if(findMacTimer)
        {
            findMacTimer->stop();
        }
        threadRunning.exchange(false);
        if(workThread)
        {
            workThread->join();
            workThread.reset();
        }
    });
}

std::string AtDeviceTask::description() const
{
    return "AtDeviceTask";
}

void AtDeviceTask::on_find_mac_time_out()
{
    stop();
    const std::string dev_dir="/dev";
    DIR *dir=nullptr;
    do{
        dir=opendir(dev_dir.c_str());
        if(!dir)break;
        struct dirent *ptr=nullptr;
        bool find=false;
        while((ptr=readdir(dir))!=nullptr)
        {
            std::string tmp=ptr->d_name;
            if(tmp.find(matchName)!=std::string::npos)
            {
                AIMY_INFO("find match device %s for match string[%s]",tmp.c_str(),matchName.c_str());
                find=true;
                matchedName=dev_dir+"/"+tmp;
                break;
            }
        }
        if(!find){
            AIMY_WARNNING("can't find match device for match[%s],retry after 1 second",matchName.c_str());
            break;
        }
        auto handle=aserial_open(matchedName.c_str());
        if(handle<=0)
        {
            AIMY_ERROR("can't open %s[%s]",matchedName.c_str(),strerror(errno));
            break;
        }
        aserial_set_opt(handle,9600,8,'N',1);
        aserial_set_rts(handle,true);
        std::string msg1("AT\r\n");
        aserial_write(handle, msg1.c_str(),msg1.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg1.size()));
        std::string msg2("AT+NAME?\r\n");
        aserial_write(handle, msg2.c_str(),msg2.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg2.size()));
        std::string msg3("AT+ADDR?\r\n");
        aserial_write(handle, msg3.c_str(),msg3.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg3.size()));
        std::string msg4("AT+PSWD?\r\n");
        aserial_write(handle, msg4.c_str(),msg4.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg4.size()));
        std::string msg5("AT+UART?\r\n");
        aserial_write(handle, msg5.c_str(),msg5.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg5.size()));
        std::string msg6("AT+ROLE?\r\n");
        aserial_write(handle, msg6.c_str(),msg6.length());
        std::this_thread::sleep_for(std::chrono::milliseconds(10*msg6.size()));
        char buf[4096];
        std::string mac_str;
        std::string psword_str="\"1234\"";
        while(1)
        {
            memset(buf,0,4096);
            auto ret=aserial_read(handle,buf,4096);
            if(ret<=0){
                AIMY_DEBUG("%s",strerror(errno));
                break;
            }
            else {
                AIMY_DEBUG("recv %s",buf);
                std::string tmp(buf,ret);
                auto pos=tmp.find("+ADDR");
                if(pos!=std::string::npos)
                {
                    char mac[64]={0};
                    if(sscanf(tmp.c_str()+pos,"+ADDR:%[^\r\n]",mac)==1)
                    {
                        mac_str=mac;
                    }
                }
                auto pos2=tmp.find("+PIN");
                if(pos!=std::string::npos)
                {
                    char pasw[64]={0};
                    if(sscanf(tmp.c_str()+pos2,"+PIN:%[^\r\n]",pasw)==1)
                    {
                        psword_str=pasw;
                    }
                }
            }
        }
        if(mac_str.empty()){
            AIMY_WARNNING("not found mac response!");
            aserial_close(handle);
            break;
        }
        AIMY_MARK("find mac[%s] pswd[%s]",mac_str.c_str(),psword_str.c_str());
        threadRunning.exchange(true);
        workThread.reset(new std::thread(std::bind(&AtDeviceTask::thread_task,this,handle)));
        notifyMac(mac_str,psword_str);
        return;
    }while(0);
    if(dir)closedir(dir);
    findMacTimer->start();
}

void AtDeviceTask::thread_task(int fd)
{
    AimyLogger::setThreadName("at_workThread");
    aserial_set_rts(fd,false);
    AIMY_DEBUG("work_thread start!");
    char buf[4096];
    while(threadRunning)
    {
        memset(buf,0,4096);
        auto ret=aserial_read(fd,buf,4096);
        if(ret>0)
        {
            AIMY_DEBUG("recv from %s %ld msg:%s",matchedName.c_str(),ret,buf);
            auto send_byte=aserial_write(fd,buf,ret);
            if(send_byte<=0)
            {
                AIMY_ERROR("send to %s failed[%s],reinit",matchedName.c_str(),strerror(errno));
                findMacTimer->start();
                break;
            }
            else {
                AIMY_DEBUG("send by  %s %ld msg:%s",matchedName.c_str(),ret,buf);
            }
        }
        else if(ret<0){
            findMacTimer->start();
            break;
        }
    }
    threadRunning.exchange(false);
    aserial_close(fd);
    AIMY_DEBUG("work_thread finished!");
}
