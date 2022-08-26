#ifndef UEVENT_MONITOR_H
#define UEVENT_MONITOR_H
#include "core/core-include.h"
#define IHOTPLUG_BUFFER_SIZE     1024
#define IHOTPLUG_NUM_ENVP        32
#define IOBJECT_SIZE         512
namespace aimy {
struct Iuevent {
    void *next;
    char buffer[IHOTPLUG_BUFFER_SIZE + IOBJECT_SIZE];
    char *devpath;
    char *action;
    char *envp[IHOTPLUG_NUM_ENVP];
};
class UeventMonitor final:public Object{
public:
    /**
     * @brief notifyUevent notify when recv a invalid uevent
     */
    Signal<std::shared_ptr<Iuevent> >notifyUevent;
public:
    UeventMonitor(TaskScheduler *parent);
    //start monitor
    bool start();
    // stop monitor
    void stop();
    ~UeventMonitor();
private:
    void on_recv();
private:
    TaskScheduler * const scheduler;
    std::shared_ptr<IoChannel> channel;
};
}
#endif // UEVENT_MONITOR_H
