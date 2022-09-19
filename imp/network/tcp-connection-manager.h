#ifndef TCPCONNECTIONMANAGER_H
#define TCPCONNECTIONMANAGER_H
#include "core/core-include.h"
#include "imp/network/base/protocal-base.h"
#include "imp/network/icache_buffer_stream.h"
namespace aimy {
class TcpConnection:public Object
{
public:
    Signal<>disconnected;
    Signal<std::shared_ptr<uint8_t> /*frame*/,uint32_t /*frame_len*/> notifyFrame;
public:
    TcpConnection(TaskScheduler *_parent,SOCKET _fd);
    virtual ~TcpConnection();
    SOCKET getFd()const { return  fd;}
    std::string getPeerHostName() const { return peerHostName;}
    uint16_t getPeerPort() const { return  peerPort;}
    virtual void setProtocal(std::shared_ptr<ProtocalBase>_protocal);
    virtual bool sendFrame(const void *frame, uint32_t frame_len);
protected:
    virtual void on_recv();
    virtual void on_write();
    virtual void on_close();
    virtual void on_error();
private:
    //signal handler
    void hand_recv(){ on_recv();}
    void hand_write(){ on_write();}
    void hand_close(){ on_close();}
    void hand_error(){ on_error();}
protected:
    TaskScheduler *const scheduler;
    const SOCKET fd;
    const std::string peerHostName;
    const uint16_t peerPort;
    std::shared_ptr<IoChannel> channel;
    std::shared_ptr<IcacheBufferBase> readCache;
    std::shared_ptr<IcacheBufferBase> writeCache;
};

/**
 * @brief The TcpConnectionManager class just use this to store connection
 */
class TcpConnectionManager:public Object
{
public:
    TcpConnectionManager(TaskScheduler *parent);
    virtual ~TcpConnectionManager();
    bool addConnection(std::shared_ptr<TcpConnection>connection);
    void removeConnection(SOCKET fd);
protected:
    TaskScheduler *const scheduler;
    std::unordered_map<SOCKET,std::shared_ptr<TcpConnection>> connetions;
};
}
#endif // TCPCONNECTIONMANAGER_H
