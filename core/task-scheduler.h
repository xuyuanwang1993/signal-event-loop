#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include"object.h"
#include "task-pipe.h"
namespace aimy{
class IoChannel;
class EventLoop;
class TaskScheduler:public Athread{
    static constexpr int64_t DEFAULT_TIMEOUT_MSEC=5000;
public:
    /**
     * @brief addChannel 添加一个socketchannel
     */
    std::shared_ptr<IoChannel> addChannel(SOCKET fd);
    virtual ~TaskScheduler();
protected:
    /**
     * @brief updateChannel 更新channel监听的事件
     * 内部存放弱引用
     */
    virtual void updateChannel(const std::shared_ptr<IoChannel>&channel)=0;
    /**
     * @brief removeChannel 移除channel
     */
    virtual void removeChannel(SOCKET fd)=0;
    virtual void handleNetworkEvent(int64_t timeout)=0;
    virtual void preinit()=0;
    virtual void resetStatus()=0;
    virtual void clearFd(SOCKET fd)=0;
protected:
    explicit TaskScheduler(const std::string &name);
    virtual void wakeup()override;
    virtual void exec() override;
    virtual std::string description() const override;
    void onWakeupChannelRecv();
    std::shared_ptr<IoChannel>checkAndGetChannelByFd(SOCKET fd);
protected:
    const std::string taskDescription;
    std::shared_ptr<Pipe>wakeupPipe;
    std::shared_ptr<IoChannel>wakeupChannel;
    std::unordered_map<SOCKET,std::weak_ptr<IoChannel>>channelsMap;
    //
    friend class IoChannel;
};
}
#endif // TASKSCHEDULER_H
