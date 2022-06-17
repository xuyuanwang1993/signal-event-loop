#ifndef AT_DEVICE_TASK_H
#define AT_DEVICE_TASK_H
#include "core/core-include.h"
#include "log/aimy-log.h"
namespace aimy {
class AtDeviceTask:public Object{
public:
    Signal<std::string,std::string>notifyMac;
public:
    AtDeviceTask(Object*parent);
    ~AtDeviceTask();
    void start(const std::string &ttyDeviceMatchName="ttyUSB");
    void stop();
    std::string description() const override;
private:
    void on_find_mac_time_out();
    void thread_task(int fd);
private:
    std::string matchName;
    std::string matchedName;
    std::shared_ptr<Timer>findMacTimer;
    std::unique_ptr<std::thread>workThread;
    bool threadRunning;
};
}

#endif // AT_DEVICE_TASK_H
