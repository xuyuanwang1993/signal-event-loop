#include "task-scheduler.h"
#include "io-channel.h"
#include "log/aimy-log.h"
#if defined(__linux) || defined(__linux__)
#include <signal.h>
#endif
using namespace aimy;

std::shared_ptr<IoChannel> TaskScheduler::addChannel(SOCKET fd)
{
    return IoChannel::create(this,fd);
}

TaskScheduler::~TaskScheduler()
{
    releaseAllRef();
    stopThread();
    wakeupChannel.reset();
    channelsMap.clear();
    wakeupPipe->close();
    wakeupPipe.reset();
}

void TaskScheduler::wakeup()
{
    char data='1';
    wakeupPipe->write(&data,1);
}

void TaskScheduler::exec()
{
    preinit();
    wakeupChannel->sync();
    while(isExecing)
    {
        handleEvent(0);
        auto timeout=getNextTimeOut();
        if(timeout<0)timeout=DEFAULT_TIMEOUT_MSEC;
        else if (timeout>DEFAULT_TIMEOUT_MSEC) {
            timeout=DEFAULT_TIMEOUT_MSEC;
        }
        handleNetworkEvent(timeout);
    }
    wakeupChannel->stop();
    resetStatus();
}

std::string TaskScheduler::description()const
{
    return taskDescription;
}

void TaskScheduler::onWakeupChannelRecv()
{
    char recv_buf;
    while(wakeupPipe->read(&recv_buf,1)>0);
}

std::shared_ptr<IoChannel> TaskScheduler::checkAndGetChannelByFd(SOCKET fd)
{
    auto iter=channelsMap.find(fd);
    if(iter==channelsMap.end())return nullptr;
    auto ret=iter->second.lock();
    if(!ret)
    {
        channelsMap.erase(iter);
        clearFd(fd);
    }
    return ret;
}

TaskScheduler::TaskScheduler(const std::string &name):taskDescription(name),wakeupPipe(new Pipe())
{

#if defined(__linux) || defined(__linux__)
    signal(SIGPIPE, SIG_IGN);
#endif

    wakeupPipe->open();
    wakeupChannel=IoChannel::create(this,wakeupPipe.get()->operator()());
    wakeupChannel->bytesReady.connect(this,std::bind(&TaskScheduler::onWakeupChannelRecv,this));
    wakeupChannel->enableReading();
    //
    notifyThreadStatus.connectFunc([](Athread::ThreadMessageType type,const std::string & msg){
        switch (type) {
        case Athread::TreadInit:
            AIMY_DEBUG("%s thread init",msg.c_str());
            break;
        case Athread::ThreadStart:
            AIMY_INFO("%s thread start",msg.c_str());
            break;
        case Athread::ThreadFinished:
            AIMY_WARNNING("%s thread finish",msg.c_str());
            break;
        }
    });
}
