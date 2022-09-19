#ifndef WEBSOCKET_API_BASE_H
#define WEBSOCKET_API_BASE_H
#include "imp/network/tcp-connection-manager.h"
#include "imp/network/websocket/protocal_websocket.h"
namespace aimy {
class WebsocketApiBase:public TcpConnection{
public:
    WebsocketApiBase(const std::string &api_name, bool is_passive, TaskScheduler *_parent, SOCKET _fd);
    virtual ~WebsocketApiBase();
    std::string description() const override
    {
        return "WS["+apiName+":"+getPeerHostName()+":"+std::to_string(getPeerPort())+"]";
    }
public:
    virtual bool sendWebsocketFrame(WS_FrameType type,const void *payload,uint32_t payload_len);
protected:
    void setProtocal(std::shared_ptr<ProtocalBase>_protocal) override;
    bool sendFrame(const void *frame, uint32_t frame_len) override;
    void on_recv() override;
    void on_write() override;
    void on_close() override;
    void on_error() override;

protected:
    virtual void handleWebsocketFrame(const StreamWebsocketFrame &frame);
    virtual void handleTextFrame(const StreamWebsocketFrame &frame);
    virtual void handlePongFrame(const StreamWebsocketFrame &frame);
    virtual void handlePingFrame(const StreamWebsocketFrame &frame);
    virtual void handleCloseFrame(const StreamWebsocketFrame &frame);
    virtual void handleOtherFrame(const StreamWebsocketFrame &frame);
protected:
    std::shared_ptr<ProtocalWebsocket> protocalWebsocket;
    bool isPassive;
    const std::string apiName;
};
}
#endif // WEBSOCKET_API_BASE_H
