#include "io-channel.h"
#include "task-scheduler.h"
#include "log/aimy-log.h"
using namespace aimy;
std::shared_ptr<IoChannel> IoChannel::create(Object *parent,SOCKET _fd)
{
    return std::shared_ptr<IoChannel>(new IoChannel(parent,_fd));
}

void IoChannel::enableReading()
{
    events|=EVENT_IN;
}

void IoChannel::enablWriting()
{
    events|=EVENT_OUT;
}

void IoChannel::disableReading()
{
    events&=~EVENT_IN;
}

void IoChannel::disableWriting()
{
    events&=~EVENT_OUT;
}

bool IoChannel::isReading()const
{
    return (events&EVENT_IN)!=EVENT_NONE;
}

bool IoChannel::isWriting()const
{
    return (events&EVENT_OUT)!=EVENT_NONE;
}

bool IoChannel::isNoneEvent()const
{
    return events==EVENT_NONE;
}

void IoChannel::rleaseFd()
{
    fd=INVALID_SOCKET;
}

SOCKET IoChannel::getFd()const
{
    return fd;
}

int IoChannel::getEvents()const
{
    return events.load();
}

void IoChannel::handleIoEvent(int _events)
{
    if(_events&EVENT_IN||_events&EVENT_PRI)
    {
        bytesReady.emit();
    }
    if (_events&EVENT_HUP) {
        closeEvent.emit();
        return;
    }
    if (_events&EVENT_ERR) {
        errorEvent.emit();
        return;
    }
    if (_events&EVENT_OUT) {
        writeReady.emit();
    }
}

IoChannel::~IoChannel()
{
    AIMY_DEBUG("release channel %d",fd);
    stop();
}

void IoChannel::sync()
{
    if(!parent)return;
    invoke(Object::getCurrentThreadId(),[this](){
        if(!parent||fd==INVALID_SOCKET)return ;
        NETWORK_UTIL::make_noblocking(fd);
        TaskScheduler *scheduler=dynamic_cast<TaskScheduler *>(parent.load());
        if(scheduler)scheduler->updateChannel(shared_from_this());
    });
}

void IoChannel::stop()
{
    if(!parent)return;
    invoke(Object::getCurrentThreadId(),[this](){
        if(!parent||fd==INVALID_SOCKET)return ;
        NETWORK_UTIL::make_noblocking(fd);
        TaskScheduler *scheduler=dynamic_cast<TaskScheduler *>(parent.load());
        if(scheduler)scheduler->removeChannel(fd);
    });
}

IoChannel::IoChannel(Object *parent,SOCKET _fd):Object(parent),bytesReady(this),writeReady(this),errorEvent(this)
  ,closeEvent(this),events(EVENT_NONE),fd(_fd)
{

}
