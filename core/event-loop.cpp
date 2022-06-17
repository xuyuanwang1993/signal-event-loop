#include "event-loop.h"
#include "epoll-task-scheduler.h"
#include "select-task-scheduler.h"
#include "io-channel.h"
#include "log/aimy-log.h"
using namespace aimy;
EventLoop::EventLoop(uint32_t nThreads, uint32_t nThreadPoolSize):
    threadPoolNotify(SelectTaskScheduler::create(-1)),threadPool(nullptr),threadPoolSize(nThreadPoolSize),index(0),running(false)
{
    if(nThreads==0)nThreads=std::thread::hardware_concurrency();
    if(nThreads<1)nThreads=1;
    AIMY_DEBUG("eventloop init threads[%u %u]",nThreads,nThreadPoolSize);
    for(uint32_t i=0;i<nThreads;++i)
    {
#if defined(__linux) || defined(__linux__)
       taskSchedulers.push_back(EpolltTaskScheduler::create(static_cast<int>(i)));
#elif defined(WIN32) || defined(_WIN32)
        taskSchedulers.push_back(SelectTaskScheduler::create(static_cast<int>(i)));
#else
        AIMY_ERROR("not supported platform!");
#endif
    }
    threadPool.reset(new ThreadPool(threadPoolNotify.get()));
}

EventLoop::~EventLoop()
{
    AIMY_WARNNING("eventloop exit enter");
    stop();
    taskSchedulers.clear();
    AIMY_WARNNING("eventloop exit");
}

void EventLoop::start()
{
    running.exchange(true);
    for(auto i:taskSchedulers)
    {
        i->startThread();
    }
    threadPoolNotify->startThread();
    threadPool->spawn(threadPoolSize);
}

void EventLoop::stop()
{
    running.exchange(false);
    threadPool->join();
    threadPoolNotify->stopThread();
    for(auto i:taskSchedulers)
    {
        i->stopThread();
    }
    AIMY_MARK("stop finished");
    std::unique_lock<std::mutex>locker(stopMutex);
    stopCv.notify_all();
}

void EventLoop::waitStop()
{
    if(!working())return;
    std::unique_lock<std::mutex>locker(stopMutex);
    stopCv.wait(locker);
}

bool EventLoop::working() const
{
    return running.load();
}

bool EventLoop::addTriggerEvent(const TriggerEvent &event)
{
    if(taskSchedulers.empty()){
        AIMY_ERROR("none eventloop taskScheduler");
        return false;
    }
    return taskSchedulers[getIndex()]->addTriggerEvent(std::forward<const TriggerEvent&>(event));
}

std::shared_ptr<Timer> EventLoop::addTimer(int64_t interval)
{
    if(taskSchedulers.empty()){
        AIMY_ERROR("none eventloop taskScheduler");
        return nullptr;
    }
    return taskSchedulers[getIndex()]->addTimer(std::move(interval));
}

std::shared_ptr<IoChannel> EventLoop::addChannel(SOCKET fd)
{
    if(taskSchedulers.empty()){
        AIMY_ERROR("none eventloop taskScheduler");
        return nullptr;
    }
    return IoChannel::create(taskSchedulers[getIndex()].get(),fd);
}

uint32_t EventLoop::getIndex()
{
    auto ret=index.load();
    ++index;
    if(index>=taskSchedulers.size())index.exchange(0);
    return ret;
}

std::shared_ptr<TaskScheduler> EventLoop::getTaskScheduler()
{
    if(taskSchedulers.empty()){
        AIMY_ERROR("none eventloop taskScheduler");
        return nullptr;
    }
    return taskSchedulers[getIndex()];
}
