#ifndef WEBSOCKET_CONNECTION_HELPER_H
#define WEBSOCKET_CONNECTION_HELPER_H
#include "imp/network/tcp-connection-manager.h"
#include "imp/network/websocket/protocal-http.h"
namespace aimy {
class WebsocketConnectionHelper:public TcpConnection{
public:
    enum WebsocketStatus:uint8_t{
        WebsocketPassive,
        WebsocketPassiveConnecting,
        WebsocketActiveConnecting,
        WebsocketConnected,
        WebsocketClosed,
    };
    Signal<std::string/*api_name*/,SOCKET/*fd*/,bool/*is_passive*/> notifyNewWSConnection;
public:
    WebsocketConnectionHelper(TaskScheduler *_parent,SOCKET _fd);
    virtual ~WebsocketConnectionHelper();
    void activeSetup(const std::string & api);
    void abort();
    void setTimeout(uint32_t time_msec);
    WebsocketStatus getStatus();
protected:
    void setProtocal(std::shared_ptr<ProtocalBase> _protocal) override;
    void passiveResponse(const std::string &client_key);
    void handleWebsocketPassiveConnection(std::shared_ptr<uint8_t> frame,uint32_t len);
    void handleActiveConnection(std::shared_ptr<uint8_t> frame,uint32_t len);
    void handleConnectSuccess(bool is_passive);
protected:
    static std::string generateSessionKey(void *ptr=nullptr);
    static std::string generateServerAcceptKey(const std::string &sesion_key);
    //sha1 handle
    static const char * sha1_get_guid(){
        return "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    }
    typedef struct {
      uint32_t state[5];
      uint32_t count[2];
      unsigned char buffer[64];
    } ws_sha1_ctx;
    static void ws_sha1_init(ws_sha1_ctx *);
    static void ws_sha1_update(ws_sha1_ctx *, const unsigned char *data, size_t len);
    static void ws_sha1_final(unsigned char digest[20], ws_sha1_ctx *);
    static void ws_hmac_sha1(const unsigned char *key, size_t key_len,
                      const unsigned char *text, size_t text_len,
                      unsigned char out[20]);
    static void ws_sha1_transform(uint32_t state[5], const unsigned char buffer[64]);
protected:
    void on_recv() override;
    void on_write() override;
    void on_close() override;
    void on_error() override;
protected:
    std::shared_ptr<ProtocalHttp> protocalHttp;
    WebsocketStatus connectionStatus;
    std::shared_ptr<Timer> timeoutTimer;
    std::string websocketApiName;
};
}
#endif // WEBSOCKET_CONNECTION_HELPER_H
