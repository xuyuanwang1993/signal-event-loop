#ifndef CANUTIL_H
#define CANUTIL_H
#include "can-type-def.h"
namespace aimy
{
/**
 * @brief The CanUtilRaw class 使用进程调用来发送数据，没有实现数据接收
 */
class CanUtilRaw :public CanControlBase
{

public:
    CanUtilRaw(Object *parent=nullptr,const std::string &can_device="can0");
    ~CanUtilRaw()override;
    /**
         * @brief sendTestControlCommand 发送单个控制命令组
         * @param cmd 待发送的控制命令组
         * @return 若无法发送，返回false，发送成功返回true
         */
    bool sendTestControlCommand(CanCommandType cmd)override;
    /**
         * @brief sendTestControlCommandList 发送多个控制命令组
         */
    bool sendTestControlCommandList(const std::list<CanCommandType>&cmdList)override;
    /**
         * @brief initCan 初始化can设备，开启can网卡，设置其工作波特率
         * @return 若失败返回false
         */
    bool initCan()override;
    /**
         * @brief startCan 启动can服务
         * @return
         */
    bool startCan()override;
    /**
         * @brief stopCan 停止can服务
         */
    void stopCan()override;
protected:
     std::list<canControlFrame> getTestCommandFrameList(CanCommandType type)override;
     void sendCanFrame(const canControlFrame&frame)override;
};
    
} // namespace aimy

#endif // CANUTIL_H
