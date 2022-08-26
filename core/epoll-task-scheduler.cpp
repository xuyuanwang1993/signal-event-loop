#include "epoll-task-scheduler.h"
#include "io-channel.h"
#include "platform.h"
#include "log/aimy-log.h"
/*epoll*/
#if defined(__linux) || defined(__linux__)
#include <sys/epoll.h>
#endif
using namespace aimy;
std::shared_ptr<TaskScheduler> EpolltTaskScheduler::create(int _id)
{
    return std::shared_ptr<TaskScheduler>(new EpolltTaskScheduler(_id));
}

EpolltTaskScheduler::~EpolltTaskScheduler()
{

}

void EpolltTaskScheduler::updateChannel(const std::shared_ptr<IoChannel>&channel)
{
#if defined(__linux) || defined(__linux__)
    if(!channel||channel->getFd()==INVALID_SOCKET)return;
    auto fd=channel->getFd();
    {
        auto iter=channelsMap.find(fd);
        if(iter==std::end(channelsMap)){
            if(!channel->isNoneEvent()){
                channelsMap.emplace(fd,std::weak_ptr<IoChannel>(channel));
                update(EPOLL_CTL_ADD, channel->getFd(),channel->getEvents());
            }
        }
        else {
            if(channel->isNoneEvent()){
                update(EPOLL_CTL_DEL, channel->getFd(),channel->getEvents());
                channelsMap.erase(iter);
            }
            else {
                update(EPOLL_CTL_MOD, channel->getFd(),channel->getEvents());
            }
        }
    }
    wakeup();
#else
    (void)channel;
#endif
}

void EpolltTaskScheduler::removeChannel(int fd)
{
#if defined(__linux) || defined(__linux__)
        auto iter=channelsMap.find(fd);
        if(iter!=std::end(channelsMap)){
            update(EPOLL_CTL_DEL, fd,0);
            channelsMap.erase(iter);
        }
    wakeup();
#else
    (void)channel;
#endif
}

void EpolltTaskScheduler::handleNetworkEvent(int64_t timeout)
{
#if defined(__linux) || defined(__linux__)
    struct epoll_event events[512];
    bzero(&events,sizeof (events));
    int numEvents = -1;
    numEvents = epoll_wait(epollFd.load(), events, 512, static_cast<int>(timeout));
    if(numEvents < 0)  //
    {
        if(aimy::platform::getErrno() != EINTR)
        {
            AIMY_ERROR("epoll error[%s]",strerror(aimy::platform::getErrno()));
        }
        else {
            AIMY_WARNNING("epoll error[%s]",strerror(aimy::platform::getErrno()));
        }
        return;
    }
    else if (numEvents>0) {
        for(int n=0; n<numEvents; n++)
        {
            auto channel=checkAndGetChannelByFd(events[n].data.fd);
            if(channel)channel->handleIoEvent(events[n].events);
        }
    }
#endif
}

void EpolltTaskScheduler::preinit()
{
#if defined(__linux) || defined(__linux__)
    epollFd.exchange(epoll_create1(0));
#endif
}

void EpolltTaskScheduler::resetStatus()
{
#if defined(__linux) || defined(__linux__)
    ::close(epollFd.load());
    epollFd.exchange(INVALID_SOCKET);
#endif
}

void EpolltTaskScheduler::clearFd(SOCKET fd)
{
#if defined(__linux) || defined(__linux__)
    update(EPOLL_CTL_DEL,fd,0);
#else
    (void)fd;
#endif
}

void EpolltTaskScheduler::update(int operation, SOCKET fd,int events)
{
#if defined(__linux) || defined(__linux__)
    struct epoll_event event ;
    bzero(&event,sizeof (event));
    if(operation != EPOLL_CTL_DEL)
    {
        event.data.fd = fd;
        event.events = static_cast<uint32_t>(events);
    }

    if(::epoll_ctl(epollFd.load(), operation, fd, &event) < 0)
    {
        AIMY_ERROR("epoll ctl error[%s]! epollfd[%d] operation[%d:%d:%d]",strerror(platform::getErrno()),epollFd.load(),operation,fd,events);
    }
#else
    (void)operation;
    (void)fd;
    (void)events;
#endif
}

EpolltTaskScheduler::EpolltTaskScheduler(int _id):TaskScheduler(std::string("epoll_")+std::to_string(_id))
{

}
