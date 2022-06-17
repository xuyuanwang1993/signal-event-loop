#ifndef UDPECHOSERVER_H
#define UDPECHOSERVER_H
#include "core/core-include.h"
#include "imp/network/icache_buffer_udp.h"
#include "imp/network/protocal-normal.h"
namespace aimy {
class UdpEchoServer:public Object{
public:
    Signal<float,float>notifySpeedKb;
public:
    UdpEchoServer(TaskScheduler *parent,const std::string &ip,uint16_t port);
    ssize_t sendData(const std::string &ip,uint16_t port,const void *data,uint32_t len);
    void start();
    void stop();
    ~UdpEchoServer();
private:
    void on_read();
    void on_write();
    void on_timeout();
private:
    SOCKET fd;
    std::shared_ptr<IoChannel> channel;
    std::string localIp;
    uint16_t localPort;
    bool echoing=false;
    std::shared_ptr<ProtocalBase> protocal;
    std::shared_ptr<IcacheBufferBase> readCache;
    std::shared_ptr<IcacheBufferBase> writeCache;
    //speed
    std::shared_ptr<Timer>speedNotifyTimer;
    uint64_t recvBytes;
    uint64_t sendBytes;
};
}
#endif // UDPECHOSERVER_H
