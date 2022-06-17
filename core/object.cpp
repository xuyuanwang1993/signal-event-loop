#include "object.h"

#include <limits>
#ifdef DEBUG
 #pragma message("Object Debug")
#else
 #pragma message("Object Release")
#endif
using namespace aimy;
Object::Object(Object *_parent):parent(_parent),threadId(0),triggerEventQueue(new TriggerEventQueueForObject),isExecing(false),isTriggering(false),lastTimerId(INVALID_TIMER_ID)
{
    if(parent.load())parent.load()->addChildRef(this);
}

Object::~Object()
{
    releaseAllRef();
}

bool Object::addTriggerEvent(const TriggerEvent &event)
{
    bool ret=triggerEventQueue->addTriggerEvent(std::move(event));
    if(ret){
        active();
        if(parent.load()&&!isTriggering){
            isTriggering.exchange(true);
            parent.load()->pushExec(this);
        }
    }
    return ret;
}

size_t Object::getThreadId()const
{
    return threadId.load();
}

void Object::setThreadId(size_t id)
{
    threadId.exchange(id);
    for(auto i:childObjectSet)
    {
        i->setThreadId(id);
    }
}

void Object::setParent(Object *_parent)
{
    if(_parent==parent)return;
    parent=_parent;
    if(_parent)_parent->addChildRef(this);
}

std::string Object::description()const
{
    return "Object";
}

size_t Object::getCurrentThreadId()
{
#ifdef _WIN32
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
#if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#define SYS_gettid __NR_gettid
#endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX) || defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

void Object::wakeup()
{

}

void Object::handleEvent(int events)
{
    (void)events;
    handleTriggerQueue();
    handleChildEvent();
    handleTimerEvent();
    if(parent.load()&&(!triggerEventQueue->empty()||!timerEventsMap.empty()||!execSubObjectSet.empty()))
    {
        parent.load()->pushExec(this);
    }
    else {
        isTriggering.exchange(false);
    }
}

void Object::handleChildEvent()
{
    std::set<Object *>execSet;
    std::swap(execSet,execSubObjectSet);
    for(auto i:execSet)
    {
        i->handleEvent(0);
    }
}

void Object::handleTriggerQueue()
{
    triggerEventQueue->handleEvent();
}

void Object::handleTimerEvent()
{
    std::queue<std::shared_ptr<Timer>>timeoutQueue;
    std::list<std::shared_ptr<Timer>>cycleQueue;
    auto now=Timer::getTimeNow();
    {
        //取出所有超时事件
        auto iter=timerEventsMap.begin();
        while(iter!=timerEventsMap.end())
        {
            if(iter->first.first<=now)
            {
                timeoutQueue.push(iter->second);
                ++iter;
            }
            else {
                break;
            }
        }
        timerEventsMap.erase(timerEventsMap.begin(),iter);
    }
    while(!timeoutQueue.empty())
    {
        timeoutQueue.front()->handleTimeoutEvent();
        //maybe timeout signal will change it's workstate
        if(!timeoutQueue.front()->single()&&timeoutQueue.front()->working()){
            cycleQueue.push_back(timeoutQueue.front());
        }
        else {
            timeoutQueue.front()->setWorkState(false);
        }
        timeoutQueue.pop();
    }
    {
        //将循环执行的定时器重新加入事件集
        now=Timer::getTimeNow();
        for(auto iter=cycleQueue.begin();iter!=cycleQueue.end();++iter)
        {
            iter->get()->setNextTimeout(now);
            timerEventsMap.emplace(std::make_pair(iter->get()->getNextTimeout(),iter->get()->getTimerId()),*iter);
        }
    }
}

int64_t Object::getNextTimeOut()
{
    if(!triggerEventQueue->empty()){
        return 0;
    }
    if(timerEventsMap.empty()&&execSubObjectSet.empty())return -1;
    auto min_time_out=timerEventsMap.empty()?std::numeric_limits<int64_t>::max():(timerEventsMap.begin()->first.first-Timer::getTimeNow());
    if(min_time_out<=0){
        return 0;
    }
    for(auto i:execSubObjectSet)
    {
        auto timeout=i->getNextTimeOut();
        if(timeout==0){
            return 0;
        }
        else if (timeout>0) {
            min_time_out=min_time_out<timeout?min_time_out:timeout;
        }
    }
    return min_time_out;
}

void Object::releaseAllRef()
{

    disconnectAll();
    isTriggering.exchange(false);
    if(parent.load())parent.load()->removeChildRef(this);
    parent=nullptr;
    for(auto i:childObjectSet)
    {
        i->setParent(nullptr);
    }
    childObjectSet.clear();
    execSubObjectSet.clear();
}

void Object::active()
{
    if(parent.load())parent.load()->wakeup();
    else {
        wakeup();
    }
}

std::shared_ptr<Timer> Object::addTimer(int64_t interval)
{
    return invoke(Object::getCurrentThreadId(),[=]()->std::shared_ptr<Timer>{
        auto ret=std::make_shared<Timer>(interval,this);
        ret->setTimerId(getNextTimerId());
        if(!isValidTimerId(ret->getTimerId())){
            return nullptr;
        }
        timerMap.emplace(ret->getTimerId(),ret);
        return ret;
    });
}

bool Object::startTimer(TimerId timerId)
{
    return invoke(Object::getCurrentThreadId(),[=]()->bool{
        auto iter=timerMap.find(timerId);
        if(iter==timerMap.end())return false;
        auto timer=iter->second;
        if(timer->working())return true;
        timer->setWorkState(true);
        timer->setNextTimeout(Timer::getTimeNow());
        timerEventsMap.emplace(std::make_pair(timer->getNextTimeout(),timerId),timer);
        active();
        return true;
    });
}

void Object::stopTimer(TimerId timerId)
{
    invoke(Object::getCurrentThreadId(),[=](){
        auto iter=timerMap.find(timerId);
        if(iter==timerMap.end())return;
        iter->second->setWorkState(false);
        timerEventsMap.erase(std::make_pair(iter->second->getNextTimeout(),timerId));
        active();
    });
}

void Object::releaseTimer(TimerId timerId)
{
    invoke(Object::getCurrentThreadId(),[=](){
        auto iter=timerMap.find(timerId);
        if(iter==timerMap.end())return;
        timerEventsMap.erase(std::make_pair(iter->second->getNextTimeout(),timerId));
        //make timer invalid
        iter->second->setTimerId(INVALID_TIMER_ID);
        timerMap.erase(timerId);
        availableIdSet.insert(timerId);
        active();
    });
}

TimerId Object::getNextTimerId()
{
    auto ret=INVALID_TIMER_ID;
    if(!availableIdSet.empty())
    {
        ret=*availableIdSet.begin();
        availableIdSet.erase(availableIdSet.begin());
    }
    else {
        auto next_timer_id=lastTimerId+1;
        if(next_timer_id>=lastTimerId)
        {
            ret=next_timer_id;
            lastTimerId=ret;
        }
    }
    return ret;
}

void Object::addChildRef(Object *child)
{
    if(!child)return;
    invoke(Object::getCurrentThreadId(),[=](){
        child->setThreadId(getThreadId());
        childObjectSet.insert(child);
    });
}

void Object::removeChildRef(Object *child)
{
    if(!child)return;
    invoke(Object::getCurrentThreadId(),[=](){
        childObjectSet.erase(child);
        execSubObjectSet.erase(child);
    });
}

void Object::pushExec(Object *execObj)
{
    if(!execObj)return;
    invoke(Object::getCurrentThreadId(),[=](){
        execSubObjectSet.insert(execObj);
        //make sure the parent is active
        if(parent.load()&&!isTriggering)
        {
            isTriggering.exchange(true);
            parent.load()->pushExec(this);
        }
    });
}


void Object::disconnectAll()
{
    invoke(Object::getCurrentThreadId(),[=](){
        auto connection_map=connectionMap;
        for(auto i:connection_map)
        {
            i.first->disconnect(SignalSlotDisConnected);
        }
        connectionMap.clear();
    });
}

void Object::disconnect(ConnectionInterface *conn)
{
    if(!conn)return;
    invoke(Object::getCurrentThreadId(),[=](){
        auto iter=connectionMap.find(conn);
        if(iter!=connectionMap.end())
        {
            conn->disconnect(SignalSlotDisConnected);
            connectionMap.erase(iter);
        }

    });
}

void Object::connect(std::shared_ptr<ConnectionInterface>conn)
{
    if(!conn)return;
    invoke(Object::getCurrentThreadId(),[=](){
        connectionMap.emplace(conn.get(),conn);
    });
}

void Athread::startThread()
{
    if(isExecing)return;
    stopThread();//release finished thread
    notifyThreadStatus.emit(TreadInit,description());
    isExecing.exchange(true);
    std::lock_guard<std::mutex>locker(threadMutex);
    workThread.reset(new std::thread([this](){
        this->threadTask();
    }));
}

void Athread::stopThread()
{
    isExecing.exchange(false);
    wakeup();
    std::lock_guard<std::mutex>locker(threadMutex);
    if(workThread&&getThreadId()!=Object::getCurrentThreadId())
    {
        if(workThread->joinable())workThread->join();
        workThread.reset();
    }
}

void Athread::waitStop()
{
    if(!isExecing)return;
    std::unique_lock<std::mutex>locker(stopMutex);
    stopCv.wait(locker);
}

std::string Athread::description() const
{
    return threadName;
}

Athread::Athread(const std::string &name):Object(nullptr),notifyThreadStatus(this),threadName(name),workThread(nullptr)
{

}

Athread::~Athread()
{
    stopThread();
}

void Athread::exec()
{
    while(isExecing)
    {
        handleEvent(0);
    }
}

void Athread::threadTask()
{
    setThreadId(Object::getCurrentThreadId());
    notifyThreadStatus.emit(ThreadStart,description());
    exec();
    notifyThreadStatus.emit(ThreadFinished,description());
    setThreadId(0);
    isExecing.exchange(false);
    //handle reserve events
    handleEvent(0);
    notifyStop();
}

void Athread::notifyStop()
{
    std::lock_guard<std::mutex>locker(stopMutex);
    stopCv.notify_all();
}

bool Timer::setInterval(int64_t _interval)
{
    if(working())
    {
        return false;
    }
    std::lock_guard<std::mutex>locker(mt);
    interval=_interval;
    if(_interval<=0){
        interval=1;
    }
    return true;
}

bool Timer::setSingle(bool _single)
{
    if(working())
    {
        return false;
    }
    isSingle.exchange(_single);
    return true;
}

bool Timer::single()const
{
    return isSingle.load();
}

bool Timer::working()const
{
    return Object::isValidTimerId(timerId)&&isWorking;
}

void Timer::stop()
{
    parent->stopTimer(timerId);
}

bool Timer::start()
{
    return parent->startTimer(timerId);
}

void Timer::release()
{
    parent->releaseTimer(timerId);
}

Timer::~Timer()
{

}

Timer::Timer(int64_t _interval, Object *_parent):timeout(_parent),isSingle(false),isWorking(false),parent(_parent),interval(1)
  ,nextTimeout(0),timerId(INVALID_TIMER_ID)
{
    setInterval(_interval);
}

void Timer::handleTimeoutEvent()
{
    timeout.emit();
}
