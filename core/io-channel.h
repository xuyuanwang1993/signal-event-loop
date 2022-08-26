#ifndef IOCHANNEL_H
#define IOCHANNEL_H
#include "network-util.h"
#include "object.h"
#include<memory>
namespace aimy {
class TaskScheduler;
class SelectTaskScheduler;
class EpolltTaskScheduler;
class IoChannel:public std::enable_shared_from_this<IoChannel>,public Object
{
public:
    enum EventType
    {
        EVENT_NONE   = 0,
        EVENT_IN     = 1,
        EVENT_PRI    = 2,
        EVENT_OUT    = 4,
        EVENT_ERR    = 8,
        EVENT_HUP    = 16,
        EVENT_RDHUP  = 8192
    };
public://signals
    /**
     * @brief bytesReady emit when the channel become readable
     */
    Signal<>bytesReady;
    /**
     * @brief writeReady emit when the channel become writable
     */
    Signal<>writeReady;
    /**
     * @brief errorEvent emit when an error occurred
     */
    Signal<>errorEvent;
    /**
     * @brief closeEvent emit when the channel is closed
     */
    Signal<>closeEvent;
public:
    static std::shared_ptr<IoChannel>create(Object *parent,SOCKET _fd);
    void enableReading();
    void enablWriting();
    void disableReading();
    void disableWriting();
    bool isReading()const;
    bool isWriting()const;
    bool isNoneEvent()const;
    /**
     * @brief rleaseFd only call this when you want recycle fd
     */
    void rleaseFd();
    SOCKET getFd()const;
     ~IoChannel();
    /**
     * @brief sync 更新调度器中channel的状态
     */
    void sync();
    /**
     * @brief stop 从调度器中移除channel
     */
    void stop();
private:
    int getEvents()const;
    void handleIoEvent(int _events);
    IoChannel(Object *parent,SOCKET _fd);
private:
    std::atomic<int>events;
    SOCKET fd;
    //
    friend class TaskScheduler;
    friend class EpolltTaskScheduler;
    friend class SelectTaskScheduler;
};
}
#endif // IOCHANNEL_H
