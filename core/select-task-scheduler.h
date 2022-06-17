#ifndef SELECTTASKSCHEDULER_H
#define SELECTTASKSCHEDULER_H
#include "task-scheduler.h"
namespace aimy {
class SelectTaskScheduler final:public TaskScheduler{
public:
    static std::shared_ptr<TaskScheduler>create(int _id);
    ~SelectTaskScheduler();
private:
    void updateChannel(const std::shared_ptr<IoChannel>&channel)override;
    void removeChannel(SOCKET fd)override;
    void handleNetworkEvent(int64_t timeout)override;
    void preinit()override;
    void resetStatus()override;
    void clearFd(SOCKET fd)override;
private:
    SelectTaskScheduler(int _id);
private:
    /**
     * @brief m_read_sets 读fd集合
     */
    fd_set read_sets;
    /**
     * @brief m_write_sets 写fd集合
     */
    fd_set write_sets;
    /**
     * @brief m_exception_sets 异常fd集合
     */
    fd_set exception_sets;
    /**
     * @brief allfdSet 所有fd的集合
     */
    std::set<SOCKET> allfdSet;
};
}
#endif // SELECTTASKSCHEDULER_H
