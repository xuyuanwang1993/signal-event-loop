#include "select-task-scheduler.h"
#include "io-channel.h"
#include "platform.h"
#include "log/aimy-log.h"
using namespace aimy;
std::shared_ptr<TaskScheduler> SelectTaskScheduler::create(int _id,const std::string &name)
{
    return std::shared_ptr<TaskScheduler>(new SelectTaskScheduler(_id,name));
}

SelectTaskScheduler::~SelectTaskScheduler()
{

}

void SelectTaskScheduler::updateChannel(const std::shared_ptr<IoChannel>&channel)
{
    if(!channel||channel->getFd()==INVALID_SOCKET)return;
    {
       auto fd=channel->getFd();
       auto iter=channelsMap.find(channel->getFd());
       if(iter!=channelsMap.end())
       {
           clearFd(iter->first);
           allfdSet.erase(iter->first);
           channelsMap.erase(iter);
       }
       if(!channel->isNoneEvent())
       {
           FD_SET(fd,&exception_sets);
           if(channel->isReading())FD_SET(fd,&read_sets);
           if(channel->isWriting())FD_SET(fd,&write_sets);
           allfdSet.insert(fd);
           channelsMap.emplace(fd,std::weak_ptr<IoChannel>(channel));
       }

    }
    wakeup();
}

void SelectTaskScheduler::removeChannel(int fd)
{
    auto iter=channelsMap.find(fd);
    if(iter!=channelsMap.end())
    {
        clearFd(iter->first);
        allfdSet.erase(iter->first);
        channelsMap.erase(iter);
    }
    wakeup();
}

void SelectTaskScheduler::handleNetworkEvent(int64_t timeout)
{
    fd_set read_sets_copy;
    fd_set write_sets_copy;
    fd_set exception_sets_copy;
    FD_ZERO(&read_sets_copy);
    FD_ZERO(&write_sets_copy);
    FD_ZERO(&exception_sets_copy);
    SOCKET max_fd;
    {
        read_sets_copy=read_sets;
        write_sets_copy=write_sets;
        exception_sets_copy=exception_sets;
        max_fd=allfdSet.empty()?1:*allfdSet.rbegin();
    }
    struct timeval tv = { static_cast<__time_t>(timeout) / 1000, static_cast<__suseconds_t>(timeout) % 1000*1000 };
    int ret = select(max_fd+1, &read_sets_copy, &write_sets_copy, &exception_sets_copy, &tv);
    if(ret<0){
#if defined(__linux) || defined(__linux__)
        if(aimy::platform::getErrno()!=EINTR&&aimy::platform::getErrno()!=EAGAIN)
        {
            AIMY_ERROR("select error[%s]",strerror(aimy::platform::getErrno()));
        }
        else {
            AIMY_WARNNING("select error[%s]",strerror(aimy::platform::getErrno()));
        }
#elif defined(WIN32) || defined(_WIN32)
        int err = aimy::platform::getErrno();
        if (err == WSAEINVAL && read_sets.fd_count == 0) {err=EINTR;}
        if(err!=EINTR){
            AIMY_ERROR("select error[%d]",err);
        }
        else {
            AIMY_WARNNING("select error[%d]",err);
        }
#endif
    }
    else if (ret>0) {
        for(SOCKET fd=1;fd<=max_fd;fd++){
            auto channel=checkAndGetChannelByFd(fd);
            if(!channel)continue;
            int events=0;
            if(FD_ISSET(fd,&read_sets_copy))events|=IoChannel::EVENT_IN;
            if(FD_ISSET(fd,&write_sets_copy))events|=IoChannel::EVENT_OUT;
            if(FD_ISSET(fd,&exception_sets_copy))events|=IoChannel::EVENT_HUP;
            channel->handleIoEvent(events);
        }
    }
}

void SelectTaskScheduler::preinit()
{

}

void SelectTaskScheduler::resetStatus()
{
    allfdSet.clear();
    FD_ZERO(&read_sets);
    FD_ZERO(&write_sets);
    FD_ZERO(&exception_sets);
}

void SelectTaskScheduler::clearFd(SOCKET fd)
{
    FD_CLR(fd,&read_sets);
    FD_CLR(fd,&write_sets);
    FD_CLR(fd,&exception_sets);
}

SelectTaskScheduler::SelectTaskScheduler(int _id,const std::string &name):TaskScheduler(name.empty()?(std::string("select_")+std::to_string(_id)):name)
{
    resetStatus();
}
