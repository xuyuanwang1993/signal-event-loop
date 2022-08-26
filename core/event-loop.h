#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include "task-scheduler.h"
#include "thread-pool.h"
namespace aimy {
class EventLoop
{
public:
    EventLoop(uint32_t nThreads=1,uint32_t nThreadPoolSize=0);
    ~EventLoop();
    void start();
    void stop();
    void waitStop();
    bool working()const;
    /**
     * @brief addTriggerEvent 添加一个即时事件
     */
    bool addTriggerEvent(const TriggerEvent &event);
    /**
     * @brief addTimer 添加一个定时器
     * 内部存放弱引用
     */
    std::shared_ptr<Timer> addTimer(int64_t interval);
    /**
     * @brief addChannel 添加一个socketchannel
     */
    std::shared_ptr<IoChannel> addChannel(SOCKET fd);
    /**
     * @brief getTaskScheduler 获取一个任务调度器
     */
    std::shared_ptr<TaskScheduler> getTaskScheduler();
    std::shared_ptr<ThreadPool> getThreadPool()const
    {
        return threadPool;
    }
private:
    uint32_t getIndex();
private:
    std::vector<std::shared_ptr<TaskScheduler>>taskSchedulers;
    std::shared_ptr<TaskScheduler> threadPoolNotify;
    std::shared_ptr<ThreadPool> threadPool;
    const uint32_t threadPoolSize;
    std::atomic<uint32_t> index;
    std::atomic<bool> running;
    std::mutex stopMutex;
    std::condition_variable stopCv;
};
}

#endif // EVENTLOOP_H
