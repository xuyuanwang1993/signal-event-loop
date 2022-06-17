#ifndef CANSERIALPROXY_H
#define CANSERIALPROXY_H
#include "imp/common/serial_utils.h"
#include "core/core-include.h"
#include "can-type-def.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#ifdef DEBUG
extern std::atomic<uint64_t>can_recv_cnt;
extern std::atomic<uint64_t>can_send_cnt;
#endif
namespace aimy {
class CanUtilSocket;
class CanSerialProxy : public Object
{
public:
    Signal<std::shared_ptr<can_frame>>notifyRecvFrame;
public:
    CanSerialProxy(CanUtilSocket *parent,std::string dev_nam,uint32_t baund_rate=1500000,uint32_t can_baund_rante=200000);
    void initSerialProxy();
    void enableWrite();
    ~CanSerialProxy();
    void init_statistics_state(bool send_state,bool recv_state);
private:
    void on_error();
    void on_close();
    void on_recv();
    void on_write();
private:
    CanUtilSocket *const canSocketSrc;
    std::string proxySerialName;
    uint32_t serialBaundRate;
    uint32_t canBaundRate;
    std::shared_ptr<Timer>reinitTimer;
    std::shared_ptr<IoChannel>workChannel;
    std::list<std::string>initDataList;
    bool send_state_statistics=false;
    bool recv_state_statistics=false;
};
}
#endif // CANSERIALPROXY_H
