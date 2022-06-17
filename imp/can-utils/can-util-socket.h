#ifndef CANUTILSOCKET_H
#define CANUTILSOCKET_H
#include "core/core-include.h"
#include "can-util.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include "can-serial-proxy.h"
namespace aimy {
enum CanDeviceStatus:uint8_t{
    CanDeviceStart,//can已启动
    CanDeviceStop,//can模块停止
    CanDeviceReconnecting,//can模块正在等待重新初始化
};

/**
 * @brief The CanUtilSocket class
 * 1.支持数据接收
 * 2.使用socketcan进行数据发送
 */
class CanUtilSocket:public CanUtilRaw{
    friend class CanSerialProxy;
public:
    /**
     * @brief notifyCanDeviceWorkSate can设备状态通知
     */
    Signal<CanDeviceStatus>notifyCanDeviceWorkSate;
    /**
     * @brief notifyDeviceDisconnected can设备连接断开通知
     */
    Signal<>notifyDeviceDisconnected;
    /**
     * @brief notifyFrameCountForTest 测试接口，统计帧的收发次数
     * total touch_screen board
     */
    Signal<uint32_t,uint32_t,uint32_t>notifyFrameCountForTest;
    /**
     * @brief notifyCanDeviceErroForTest can出现异常时调用此接口
     */
    Signal<>notifyCanDeviceErroForTest;
public:
    /**
     * @brief CanUtilSocket
     * @param reinitThresholdMsec 设备出现异常时，重新初始化的间隔
     */
    CanUtilSocket(TaskScheduler *parent,uint32_t reinitThresholdMsec,const std::string &can_device="can0");
    ~CanUtilSocket();
    bool startCan()override;
    void stopCan()override;
    bool sendBroadCastCommand(BroadCastCommand cmd)override;
    bool sendLightControlCommand(CanDeviceAddr addr,CanLightOperationType type,uint8_t data ,bool withData)override;
    bool sendPillarControlCommand(CanDeviceAddr addr,PillarCmdType type,uint8_t data ,bool withData)override;
    //向指定硬件地址发送指定数据
    virtual bool sendFrameToSpecificPhysicalAddr(uint16_t physicalAddr,std::shared_ptr<uint8_t>data,uint32_t len);
    static std::pair<std::shared_ptr<uint8_t>,uint32_t>convertHexstrToBytes(const void *hexByteBuf,uint32_t len);
    void initCanProxy(std::string dev_nam,uint32_t baund_rate=1500000,uint32_t can_baund_rante=200000);
    CanSerialProxy *getCanproxy(){return proxy.get();};
public:
    //test
    void controlPacketUnitTest(CanDeviceAddr addr,uint8_t data1,uint8_t data2,uint32_t data_len);
protected:
    void sendCanFrame(const canControlFrame&frame)override;
protected://slots
    void on_error();
    void on_close();
    void on_recv();
    void on_write();
    void on_reconnect();
    void on_send_timeout();
protected:
    void handCanFrame(const can_frame &frame);
    void setDeviceState(CanDeviceStatus status);
    bool initCanSocket();
    void reconnect();
    std::shared_ptr<canControlFrame> getFrameCache(uint32_t canId);
    void initCanFrameCache();
    void initCanFramCacheByAddr(CanPhysicalAddr addr);
protected:
    static constexpr uint32_t ApplicationMessageHeaderLen=6;
    //protocal parse
    void handleMessage(const canControlFrame &data);
    std::string convertHexBuf(const uint8_t *data,uint32_t len);
    void handleApplicationMessage(const canControlFrame &data,uint16_t sendAddr);
    //touch screen
    void handleTouchScreenMessage(const canControlFrame &data,uint16_t sendAddr);
    //board
    void handle1PHandSensor(const canControlFrame &data,uint16_t sendAddr);
    void handle2PHandSensor(const canControlFrame &data,uint16_t sendAddr);
    void handle1PFootSensor(const canControlFrame &data,uint16_t sendAddr);
    void handle2PFootSensor(const canControlFrame &data,uint16_t sendAddr);
    bool isValidBoardState(uint8_t byte);
    bool isBoardTestData(uint8_t byte);
    void handleSensorState(uint8_t byte, const std::list<uint8_t> &indexList, const std::list<uint8_t> &maskList);
    //
    void handleHardwareTest(const canControlFrame &data,uint16_t sendAddr);
    //
    void handleInertCoin(const canControlFrame &data,uint16_t sendAddr);
protected:
    TaskScheduler *const scheduler;
    std::shared_ptr<IoChannel>workChannel;
    std::shared_ptr<Timer>reinitTimer;
    std::shared_ptr<Timer>sendTimeoutTimer;
    std::shared_ptr<Timer>recvTimeoutTimer;
    std::atomic<CanDeviceStatus>deviceStatus;
    std::list<canControlFrame>sendDataList;
    sockaddr_can addr;
    std::unordered_map<uint32_t,std::shared_ptr<canControlFrame>>canFrameCache;
    uint32_t recvFrameCounts;
    uint32_t touchScreenFrames;
    uint32_t boardFrames;
    //
    std::shared_ptr<CanSerialProxy>proxy;
};
}
#endif // CANUTILSOCKET_H
