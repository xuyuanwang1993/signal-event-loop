#ifndef SERVIEUDPTEST_H
#define SERVIEUDPTEST_H
#include "core/core-include.h"
namespace aimy {
class UdpTestClient:public Object{
public:
    Signal<std::string ,uint16_t,std::shared_ptr<uint8_t>,uint32_t >notifyMessage;
public:
    UdpTestClient(TaskScheduler *parent,const std::string &ip,uint16_t port);
    ssize_t sendData(const std::string &ip,uint16_t port,const void *data,uint32_t len);
    void start();
    void stop();
    ~UdpTestClient();
private:
    void on_read();
private:
    SOCKET fd;
    std::shared_ptr<IoChannel> channel;
    std::string localIp;
    uint16_t localPort;
    std::atomic<bool>working;
};
}
#endif // SERVIEUDPTEST_H
