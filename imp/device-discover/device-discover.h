#ifndef DEVICEDISCOVER_H
#define DEVICEDISCOVER_H
#include "core/core-include.h"
#include "third_party/json/cjson-interface.h"
#define DEFAULT_AGENT "aimy"
namespace aimy {
using neb::CJsonObject;
class DeviceDiscover:public Object
{
public:
    Signal<std::string>notifyMessage;
public:
    DeviceDiscover(TaskScheduler *parent,const std::string & agent,bool enableRecv,const std::string &ip="239.255.255.249",uint16_t port=8090);
    void sendDiscover(std::string match_string, std::string specificIp="");
    void sendData(const void *data,uint32_t len,const std::string&des_ip,uint16_t des_port);
    void sendData(const void *data,uint32_t len,const struct sockaddr_in &addr);
    void start(std::shared_ptr<ThreadPool> pool);
    void stop();
    void setUpdateTimerState(bool enable=false);
    ~DeviceDiscover();
private:
    void on_read();
    void handleMessage(const void *data,uint32_t len,const struct sockaddr_in &addr);
    static void packetBufWithShellCommand(const std::string &shellcommand,CJsonObject &object,const std::string & name);
    void on_timeout();
private:
    std::shared_ptr<ThreadPool> threadPool;
    const std::string userAgent;
    const std::string multicastip;
    const uint16_t multicastPort;
    SOCKET fd;
    std::shared_ptr<IoChannel> channel;
    std::shared_ptr<Timer>updateTimer;
    std::set<std::string>localIpSet;
};
}
#endif // DEVICEDISCOVER_H
