#ifndef EPOLLTASKSCHEDULER_H
#define EPOLLTASKSCHEDULER_H
#include "task-scheduler.h"
namespace aimy {
class EpolltTaskScheduler final:public TaskScheduler{
public:
    static std::shared_ptr<TaskScheduler>create(int _id);
    ~EpolltTaskScheduler();
protected:
    void updateChannel(const std::shared_ptr<IoChannel>&channel)override;
    void removeChannel(SOCKET fd)override;
    void handleNetworkEvent(int64_t timeout)override;
    void preinit()override;
    void resetStatus()override;
    void clearFd(SOCKET fd)override;
private:
    EpolltTaskScheduler(int _id);
    void update(int operation, SOCKET fd,int events);
private:
    std::atomic<SOCKET> epollFd;
};
}
#endif // EPOLLTASKSCHEDULER_H
