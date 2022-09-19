/**
*compile with std=c++11
* link with  -pthread
* */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <mutex>
#include <condition_variable>
#include<thread>
#include<future>
#include<vector>
#include<deque>
#include<queue>//priority_queue
#include<functional>//bind
#include<assert.h>
#include"object.h"
namespace aimy {
template <typename _Callable, typename... _Args>
using invoke_result_t=typename std::result_of< _Callable( _Args...)>::type;
using clock_t = std::chrono::steady_clock;
class ThreadPool;
namespace concurrent {
enum TaskState:uint8_t{
    TASK_INIT,
    TASK_WAIT_START,
    TASK_CANCELLED,
    TASK_RUNNING,
    TASK_FINISHED,
};
}
class ThreadPoolFutureInterface{
    friend class ThreadPool;
public:
    Signal<concurrent::TaskState>notifyStatus;
    Signal<std::string>notifyException;
public:
    bool cancelTask()
    {
        if(runningState.load()==concurrent::TASK_WAIT_START)
        {
            setStatus(concurrent::TASK_CANCELLED);
            return true;
        }
        else {
            return false;
        }
    }

    concurrent::TaskState taskStatus()const
    {
        return runningState.load();
    }

    virtual ~ThreadPoolFutureInterface(){

    }
protected:
    explicit ThreadPoolFutureInterface(Object *parent):notifyStatus(parent)
      ,notifyException(parent),runningState(concurrent::TASK_INIT){
    }
    void run()
    {
        if(runningState.load()==concurrent::TASK_WAIT_START)
        {
            setStatus(concurrent::TASK_RUNNING);
            runFunction();
        }
        setStatus(concurrent::TASK_FINISHED);
    }
    virtual void runFunction()=0;

    void setStatus(concurrent::TaskState status)
    {
        if(status!=runningState.load())
        {
            runningState.exchange(status);
            notifyStatus.emit(status);
        }
    }
    std::atomic<concurrent::TaskState>runningState;
};
template <typename RetT>
class ThreadPoolFuture;
template <typename _Callable, typename... _Args>
using threadpool_ret_type=std::shared_ptr<ThreadPoolFuture<invoke_result_t<_Callable, _Args...>>>;
template <typename RetT>
class ThreadPoolFuture:public ThreadPoolFutureInterface
{
    friend class ThreadPool;
public:
    Signal<RetT>notifyResult;
public:
    std::future<RetT>futrue()
    {
        return rawResult.get_future();
    }
protected:
    ThreadPoolFuture(Object *parent,const std::function<RetT()>&task):ThreadPoolFutureInterface(parent),notifyResult(parent),runTask(task)
    {

    }
    void runFunction()override
    {
        RetT result;
        try {
            result=runTask();
        } catch (const std::exception &e) {
            ThreadPoolFutureInterface::notifyException(e.what());
        } catch(const std::string &e){
            ThreadPoolFutureInterface::notifyException(e);
        }catch(...) {
            ThreadPoolFutureInterface::notifyException("undefined exception");
        }
        rawResult.set_value(result);
        notifyResult.emit(result);
    }

    const std::function<RetT()>runTask;
    std::promise<RetT>rawResult;
};
//特例化void返回值的处理
template <>
class ThreadPoolFuture<void>:public ThreadPoolFutureInterface
{
    friend class ThreadPool;
public:
    Signal<>notifyResult;
public:
    std::future<void>futrue()
    {
        return rawResult.get_future();
    }
protected:
    ThreadPoolFuture(Object *parent,const std::function<void()>&task):ThreadPoolFutureInterface(parent),notifyResult(parent),runTask(task)
    {

    }
    void runFunction()override
    {
        try {
            runTask();
        } catch (const std::exception &e) {
            ThreadPoolFutureInterface::notifyException(e.what());
        } catch(const std::string &e){
            ThreadPoolFutureInterface::notifyException(e);
        }catch(...) {
            ThreadPoolFutureInterface::notifyException("undefined exception");
        }
        rawResult.set_value();
        notifyResult.emit();
    }
    const std::function<void()>runTask;
    std::promise<void>rawResult;
};
class ThreadPool final{
public:
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    int count() const;
    uint32_t taskCount() const;
    void spawn(int count = 1);
    void join();
    void run(uint32_t index);
    bool runOne();

    template <typename _Callable, typename... _Args>
    auto enqueue(_Callable &&f, _Args &&...args) -> threadpool_ret_type<_Callable,_Args ...>{
        return schedule(clock_t::now(), std::forward<_Callable>(f), std::forward<_Args>(args)...);
    }

    template <typename _Callable, typename... _Args>
    auto schedule(clock_t::duration delay, _Callable &&f, _Args &&...args) -> threadpool_ret_type<_Callable,_Args ...>
    {
        return schedule(clock_t::now() + delay, std::forward<_Callable>(f), std::forward<_Args>(args)...);
    }


    template <typename _Callable, typename... _Args>
    auto schedule(uint32_t priority, _Callable &&f, _Args &&...args) -> threadpool_ret_type<_Callable,_Args ...>
    {
        if(priority>10)priority=10;
        auto time_point=clock_t::now()-std::chrono::hours(10)+std::chrono::hours(priority);
        return schedule(time_point, std::forward<_Callable>(f), std::forward<_Args>(args)...);
    }


    template <typename _Callable, typename... _Args>
    auto schedule(clock_t::time_point time, _Callable &&f, _Args &&...args) -> threadpool_ret_type<_Callable,_Args ...>
    {
        using RetT=invoke_result_t<_Callable, _Args...>;
        std::unique_lock<std::mutex> lock(mTasksMutex);
        auto bound = [=]()->RetT{
            return f(args ...);
        };
        std::shared_ptr<ThreadPoolFuture<RetT>>ret( new ThreadPoolFuture<RetT>(parent,bound));
        ret.get()->setStatus(concurrent::TASK_WAIT_START);
        mTasks.push({time, [ret]()->void{ ret.get()->run();}});
        mCondition.notify_one();
        return ret;
    }
    template <typename _Callable, typename... _Args>
    auto buildTask(_Callable &&f, _Args &&...args) -> threadpool_ret_type<_Callable,_Args ...>
    {
        using RetT=invoke_result_t<_Callable, _Args...>;
        auto bound = [=]()->RetT{
            return f(args ...);
        };
        std::shared_ptr<ThreadPoolFuture<RetT>>ret( new ThreadPoolFuture<RetT>(parent,bound));
        return ret;
    }

    void pushTask(std::shared_ptr<ThreadPoolFutureInterface>task)
    {
        if(task.get()->taskStatus()!=concurrent::TASK_INIT)return;
        task.get()->setStatus(concurrent::TASK_WAIT_START);
        std::unique_lock<std::mutex> lock(mTasksMutex);
        mTasks.push({clock_t::now(), [task]()->void{ task.get()->run();}});
        mCondition.notify_one();
    }

    ThreadPool(Object *_parent);
    ~ThreadPool();
protected:
    std::function<void()> dequeue(); // returns null function if joining

private:
    std::vector<std::thread> mWorkers;
    std::atomic<bool> mJoining {false};

    struct Task {
        clock_t::time_point time;
        std::function<void()> func;
        bool operator>(const Task &other) const { return time > other.time; }
        bool operator<(const Task &other) const { return time < other.time; }
    };
    std::priority_queue<Task, std::deque<Task>, std::greater<Task>> mTasks;

    mutable std::mutex mTasksMutex, mWorkersMutex;
    std::condition_variable mCondition;
     Object *const parent;
};

}


#endif // THREADPOOL_H
