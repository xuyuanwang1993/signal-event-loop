#ifndef OBJECT_H
#define OBJECT_H
#include <thread>
#include<atomic>
#include<set>
#include<future>
#include<functional>
#include<map>
#include<unordered_map>
#include<list>
#include<functional>
#include<queue>
#if defined(__linux) || defined(__linux__)
#include<syscall.h>
#include<unistd.h>
#elif defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#endif
/*
 * 异步编程注意事项
 * 1. 异步回调传递需考虑变量生命周期，优先传值，若需传递指针或者引用，需保证其生命周期大于执行周期，或者传递弱引用
 * 2. 回调中不得执行过于耗时的操作，耗时操作需使用额外的线程执行
 * 3. 回调中若需要释放资源，不得立即释放与回调来源相关的资源，若需要释放，需要利用回调源提供的接口延迟释放
 * 4. 回调中引用回调对象自身时应该使用弱引用或者原始指针，不得使用共享指针，可能造成循环引用,为避免此问题，可将回调对象作为回调函数入参
 */
#define INVALID_TIMER_ID 0
namespace aimy {
typedef uint32_t TimerId;
using TriggerEvent=std::function<void()>;
template <typename _Callable, typename... _Args>
using invoke_result_t=typename std::result_of< _Callable( _Args...)>::type;
template <typename _Callable, typename... _Args>
using invoke_future_t = std::future<invoke_result_t<_Callable,_Args ...>>;
class TriggerEventQueueForObject
{
public:
    explicit TriggerEventQueueForObject(uint32_t _capacity=100000):capacity(_capacity)
    {

    }
    bool addTriggerEvent(const TriggerEvent &event)
    {
        std::lock_guard<std::mutex>locker(mt);
        if(cache.size()>=capacity)return false;
        cache.push(std::move(event));
        return true;
    }
    void handleEvent()
    {
        std::queue<TriggerEvent> execQueue;
        {
            std::lock_guard<std::mutex>locker(mt);
            std::swap(execQueue,cache);
        }
        while(!execQueue.empty())
        {
            execQueue.front()();
            execQueue.pop();
        }
    }
    bool empty()const{std::lock_guard<std::mutex>locker(mt);return  cache.empty();}
    ~TriggerEventQueueForObject()
    {

    }
private:
    const uint32_t capacity;
    mutable std::mutex mt;
    std::queue<TriggerEvent> cache;
};
struct SignalInterface;
class Object;
template<typename... _Args>
using slot_func_type= std::function<void(_Args ...)>;
template<typename... _Args>
struct Connection;
template<typename... _Args>
struct Signal;
enum SignalConnectionStatus:uint8_t{
    SignalNormal=0,
    SignalDisconnected=1,
    SignalSlotDisConnected=2,
    SignalAllDisconneted=3,
};

struct ConnectionInterface{
    std::atomic<int>status;
    Object *receiver;
    virtual void disconnect(SignalConnectionStatus flag=SignalNormal)=0;
    virtual ~ConnectionInterface(){}
    ConnectionInterface(Object *recvObject):status(SignalNormal),receiver(recvObject){}
};

struct SignalInterface{
    virtual void disconnect(ConnectionInterface *conn)=0;
    virtual void disconnectAll()=0;
    virtual ~SignalInterface(){}
};
class Timer;
class Object
{
public:
    explicit Object(Object *_parent);
    virtual ~Object();
    template <typename _Callable, typename... _Args>
    auto invoke(size_t threadId,_Callable &&f, _Args &&...args) -> invoke_result_t<_Callable,_Args ...>
    {
        if(threadId==0)threadId=getCurrentThreadId();
        if(threadId!=getThreadId())
        {
            auto ret=addSyncEvent(f,std::forward<_Args>(args)...);
            ret.wait();
            std::future<int> test_f;
            return ret.get();
        }
        else {
            auto bound=std::bind(f,std::forward<_Args>(args) ...);
            return bound();
        }
    }
    template <typename _Callable, typename... _Args>
    auto addSyncEvent(_Callable &&f, _Args &&...args)  -> invoke_future_t<_Callable, _Args...>
    {
        using R=invoke_result_t<_Callable,_Args ...>;
        auto bound = std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...);
        auto task = std::make_shared<std::packaged_task<R()>>([bound]() {
            try {
                return bound();
            } catch (const std::exception &e) {
                /*run failed,print the exception string*/
                //printf("%s\r\n",e.what());
                throw;
            }
        });
        std::future<R> result = task->get_future();
        do{
            if(execing())
            {
                auto ret=addTriggerEvent([task](){
                    (*task)();
                });
                if(ret){
                    break;
                }
            }
            (*task)();
        }while(0);
        return result;
    }
    std::shared_ptr<Timer>addTimer(int64_t interval);
    virtual bool addTriggerEvent(const TriggerEvent &event);
    size_t getThreadId()const;
    void setThreadId(size_t id);
    void setParent(Object *_parent);
    virtual std::string description()const;
    bool execing()const{return isExecing.load()||threadId.load()!=0;}
public:
    static size_t getCurrentThreadId();
protected:
    virtual void wakeup();
    virtual void handleEvent(int events);
    void handleChildEvent();
    void handleTriggerQueue();
    void handleTimerEvent();
    int64_t getNextTimeOut();
    void releaseAllRef();
    void active();
private:
    bool startTimer(TimerId timerId);
    void stopTimer(TimerId timerId);
    void releaseTimer(TimerId timerId);
    TimerId getNextTimerId();
    static bool isValidTimerId(const TimerId &id){return id!=INVALID_TIMER_ID;}
private:
    /**
     * @brief addChildRef
     */
    void addChildRef(Object *child);
    /**
     * @brief removeChildRef 仅在析构时调用
     */
    void removeChildRef(Object *child);

    void pushExec(Object *execObj);
protected:
    void disconnectAll();
private:
    void disconnect(ConnectionInterface *conn);
    void connect(std::shared_ptr<ConnectionInterface>conn);
protected:
    std::atomic<Object *>parent;
    std::atomic<size_t> threadId;
    std::set<Object *>childObjectSet;
    std::set<Object *>execSubObjectSet;
    std::shared_ptr<TriggerEventQueueForObject>triggerEventQueue;
    std::atomic<bool> isExecing;
    std::atomic<bool> isTriggering;
    std::map<ConnectionInterface*,std::shared_ptr<ConnectionInterface>>connectionMap;
    //timer
    uint32_t lastTimerId;
    std::set<TimerId>availableIdSet;
    std::unordered_map<TimerId,std::shared_ptr<Timer>>timerMap;
    std::map<std::pair<int64_t,TimerId>,std::shared_ptr<Timer>>timerEventsMap;
    template <typename... _Args>
    friend struct Connection;
    template <typename... _Args>
    friend struct Signal;
    friend class Timer;
};
template <typename... _Args>
struct Connection:public ConnectionInterface{
    ~Connection(){};
    explicit Connection(Signal<_Args...> *signalObject,Object *recvObject,const slot_func_type<_Args...>&f)
        :ConnectionInterface(recvObject),sender(signalObject),func(f){
    }
    Signal<_Args...> * sender;
    slot_func_type<_Args...> func;
    void operator()(_Args ... a)
    {
        if(!func)return ;
        func(std::forward<_Args>(a)...);
    }
    void disconnect(SignalConnectionStatus flag=SignalNormal)override
    {
        status.exchange(status.load()|flag);
        auto state=status.load();
        if((state&SignalDisconnected)==0)
        {
            status.exchange(status.load()|SignalDisconnected);
            sender->disconnect(this);
        }
        if((state&SignalSlotDisConnected)==0)
        {
            status.exchange(status.load()|SignalSlotDisConnected);
            if(receiver)receiver->disconnect(this);
        }
    }
};

template<typename... _Args>
struct Signal:public SignalInterface{
public:
    std::shared_ptr<ConnectionInterface> connectFunc(const slot_func_type<_Args...>&f)
    {
        return signalObject->invoke(Object::getCurrentThreadId(),[=](){
            auto connection=std::make_shared<Connection<_Args ...>>(this,nullptr,f);
            connectionList.push_back(connection);
            return connection;
        });
    }
    std::shared_ptr<ConnectionInterface> connect(Object *recvObject,const slot_func_type<_Args...> &f)
    {
        return signalObject->invoke(Object::getCurrentThreadId(),[=](){
            auto connection=std::make_shared<Connection<_Args ...>>(this,recvObject,f);
            connectionList.push_back(connection);
            if(recvObject)recvObject->invoke(signalObject->getThreadId(),[connection,recvObject](){
                recvObject->connect(connection);
            });
            return connection;
        });
    }

    void emit(_Args ... a)
    {
        signalObject->invoke(Object::getCurrentThreadId(),[=](){
            auto execList=connectionList;
            for(auto i:execList)
            {
                auto bound = std::bind(i->func, a ...);
                if(i->receiver)
                {
                    i->receiver->addSyncEvent([bound](){
                        bound();
                    });
                }
                else {
                    bound();
                }
            }
        });
    }
    void operator()(_Args ... a)
    {
        signalObject->invoke(Object::getCurrentThreadId(),[=](){
            auto execList=connectionList;
            for(auto i:execList)
            {
                if(i->status!=SignalNormal)continue;
                auto bound = std::bind(i->func, a ...);
                if(i->receiver)
                {
                    i->receiver->addSyncEvent([bound](){
                        bound();
                    });
                }
                else {
                    bound();
                }
            }
        });
    }

    void disconnect(ConnectionInterface *conn)override
    {
        signalObject->invoke(Object::getCurrentThreadId(),[=](){
            for(auto iter=connectionList.begin();iter!=connectionList.end();++iter)
            {
                auto ptr=(*iter).get();
                if(ptr==conn)
                {
                    conn->disconnect(SignalDisconnected);
                    connectionList.erase(iter);
                    break;
                }
            }
        });
    }

    void disconnectAll()override
    {
        signalObject->invoke(signalObject->getThreadId(),[=](){
            auto operationList=connectionList;
            for(auto i:operationList)
            {
                if(i.get()->receiver!=nullptr)
                {
                    i.get()->disconnect(SignalDisconnected);
                }
            }
            connectionList.clear();
        });
    }
    Signal(Object *_object):signalObject(_object){}
    ~Signal(){disconnectAll();}

private:
    Object *const signalObject;
    std::list<std::shared_ptr<Connection<_Args ...>>>connectionList;
};

class Athread:public Object
{
public:
    enum ThreadMessageType{
        TreadInit,
        ThreadStart,
        ThreadFinished,
    };
    Signal<ThreadMessageType,const std::string &>notifyThreadStatus;
public:
    void startThread();
    void stopThread();
    void waitStop();
    std::string description() const override;
    Athread(const std::string&name="defaultThread");
    virtual ~Athread();
protected:
    virtual void exec();
private:
    void threadTask();
    void notifyStop();
private:
    const std::string threadName;
    std::mutex threadMutex;
    std::unique_ptr<std::thread>workThread;
    std::mutex stopMutex;
    std::condition_variable stopCv;
};

class Timer{
public:
    Signal<>timeout;
    bool setInterval(int64_t _interval);
    bool setSingle(bool _single);
    bool single()const;
    bool working()const;
    int64_t getInterval()const{return  interval;}
    void stop();
    bool start();
    /**
     * @brief release must call release while you don't need this timer
     * after calling this function,this timer maybe invalid
     */
    void release();
    ~Timer();
    Timer(int64_t _interval, Object *_parent);
public:
    /**
     * @brief getTimeNow 获取从1970年到当前时间
     * @return
     */
    template<class ChronoTimeType=std::chrono::milliseconds>
    static int64_t getTimeNow(){
        auto timePoint = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<ChronoTimeType>(timePoint.time_since_epoch()).count();
    }
protected:
    /**
     * @brief setNextTimeout 设置定时器到期时间
     * @param currentTimePoint 当前时间
     */
    void setNextTimeout(int64_t currentTimePoint)
    {
        nextTimeout = currentTimePoint + interval;
    }
    /**
     * @brief getNextTimeout 获取定时器到期时间
     * @return
     */
    int64_t getNextTimeout() const
    {
        return nextTimeout;
    }
    TimerId getTimerId()const
    {
        return timerId;
    }
    void setTimerId(TimerId _id)
    {
        timerId=_id;
    }
    void setWorkState(bool _isworking)
    {
        isWorking.exchange(_isworking);
    }
    virtual void handleTimeoutEvent();
protected:
    std::mutex mt;
    std::atomic<bool>isSingle;
    std::atomic<bool>isWorking;
    Object *const parent;
    int64_t interval;
    int64_t nextTimeout;
    std::atomic<TimerId> timerId;
    //
    friend class Object;
};
}
#endif // OBJECT_H
