#include "can-util-socket.h"
#define MaxCanFrameCacheSize 8192
using namespace aimy;
CanUtilSocket::CanUtilSocket(TaskScheduler *parent,uint32_t reinitThresholdMsec, const std::string &can_device):CanUtilRaw(parent,can_device),notifyCanDeviceWorkSate(this)
  ,notifyDeviceDisconnected(this),notifyFrameCountForTest(this),notifyCanDeviceErroForTest(this)
  ,scheduler(parent),workChannel(nullptr),deviceStatus(CanDeviceStop),recvFrameCounts(0),touchScreenFrames(0)
  ,boardFrames(0),proxy(nullptr)
{
    reinitTimer=parent->addTimer(reinitThresholdMsec);
    reinitTimer->setSingle(true);
    reinitTimer->timeout.connect(this,std::bind(&CanUtilSocket::on_reconnect,this));

    sendTimeoutTimer=parent->addTimer(2000);
    sendTimeoutTimer->setSingle(true);
    reinitTimer->timeout.connect(this,std::bind(&CanUtilSocket::on_send_timeout,this));

    recvTimeoutTimer=parent->addTimer(200);
    recvTimeoutTimer->setSingle(true);
    recvTimeoutTimer->timeout.connectFunc([this](){
        AIMY_ERROR("can recv timeout error!");
        this->notifyCanDeviceErroForTest();
    });
    initCanFrameCache();
}

CanUtilSocket::~CanUtilSocket()
{
    reinitTimer->release();
    stopCan();
}

bool CanUtilSocket::startCan()
{
    AIMY_DEBUG("start can");
    return invoke(Object::getCurrentThreadId(),[=](){
        do{
            if(deviceStatus!=CanDeviceStop)return deviceStatus==CanDeviceStart;
            if(!initCanSocket())break;
            setDeviceState(CanDeviceStart);
            return true;
        }while(0);
        reconnect();
        return false;
    });

}

void CanUtilSocket::stopCan()
{
    AIMY_DEBUG("stop can");
    invoke(Object::getCurrentThreadId(),[=](){
        if(deviceStatus==CanDeviceReconnecting)
        {
            setDeviceState(CanDeviceStop);
            reinitTimer->stop();
        }
        if(deviceStatus!=CanDeviceStart)return ;
        if(workChannel)
        {
            workChannel->stop();
            NETWORK_UTIL::close_socket(workChannel->getFd());
        }

        workChannel.reset();

        setDeviceState(CanDeviceStop);
    });
}

bool CanUtilSocket::sendBroadCastCommand(BroadCastCommand cmd)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        uint8_t data[2]={static_cast<uint8_t>(cmd>>8),static_cast<uint8_t>(cmd&0xFF)};
        auto frame=packCanFrame(data,2,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(BROADCAST_DEVICE_ADDR)
                                ,physicalAddrToCanId(CanPhysicalAddrBroadcastGroup));
        sendCanFrame(frame);
        return true;
    });
}

bool CanUtilSocket::sendLightControlCommand(CanDeviceAddr addr,CanLightOperationType type,uint8_t data ,bool withData)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        uint8_t data_send[2]={static_cast<uint8_t>(type),data};
        auto frame=packCanFrame(data_send,withData?2:1,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(addr)
                                ,physicalAddrToCanId(static_cast<CanPhysicalAddr>(READ_CAN_PHYSICAL_ADDR(addr))));
        sendCanFrame(frame);
        return true;
    });
}

bool CanUtilSocket::sendPillarControlCommand(CanDeviceAddr addr,PillarCmdType type,uint8_t data ,bool withData)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        if(addr!=OTHERDEVICE_GROUP_ADDR_1&&addr!=OTHERDEVICE_GROUP_ADDR_2)return false;
        uint8_t data_send[2]={static_cast<uint8_t>(type),data};
        auto frame=packCanFrame(data_send,withData?2:1,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(addr)
                                ,physicalAddrToCanId(static_cast<CanPhysicalAddr>(READ_CAN_PHYSICAL_ADDR(addr))));
        sendCanFrame(frame);
        return true;
    });
}

bool CanUtilSocket::sendFrameToSpecificPhysicalAddr(uint16_t physicalAddr, std::shared_ptr<uint8_t> data, uint32_t len)
{
    return invoke(Object::getCurrentThreadId(),[=](){
        canControlFrame frame;
#ifdef DEBUG
        assert(len>0);
#endif
        if(len<=0)return false;
        frame.data_len=len+3;
        frame.data.reset(new uint8_t[frame.data_len],std::default_delete<uint8_t[]>());
        auto data_ptr=frame.data.get();
        auto remoteCanId=physicalAddrToCanId(physicalAddr);
        data_ptr[0]=0X55;
        data_ptr[1]=0xAA;
        memcpy(data_ptr+3,data.get(),len);
        uint8_t sum=0;
        for(uint8_t i=3;i<frame.data_len;++i)
        {
            sum+=data_ptr[i];
        }
        data_ptr[2]=0x100-sum;
        uint32_t index=0;
        //
        frame.optionCanIdList.push_front(remoteCanId|(index<<5));
        uint32_t offset=frame.data_len;
        while(offset>0)
        {
            frame.optionCanIdList.push_front(remoteCanId|(index<<5));
            ++index;
            if(offset<=8)break;
            offset-=8;
        }
        sendCanFrame(frame);
        return true;
    });
}

std::pair<std::shared_ptr<uint8_t>,uint32_t>CanUtilSocket::convertHexstrToBytes(const void *hexByteBuf,uint32_t len)
{
    auto convert_func=[](uint8_t byte)->uint8_t
    {
      if(byte>='a'&&byte<='f')
      {
          return byte-'a'+10;
      }
      else if (byte>='A'&&byte<='F') {
          return byte-'A'+10;
      }
      else if (byte>='0'&&byte<='9') {
          return byte-'0';
      }
      else {
          AIMY_WARNNING("false input %02x:%c",byte,byte);
          return 0;
      }
    };
    uint32_t ret_len=(len+1)/2;
    if(ret_len==0)return {nullptr,0};
    auto ptr=new uint8_t[ret_len];
    std::shared_ptr<uint8_t> buf(ptr,std::default_delete<uint8_t[]>());
    memset(ptr,0,ret_len);
    const uint8_t *data_ptr=static_cast<const uint8_t *>(hexByteBuf);
    for(uint32_t index=0;index<ret_len;++index)
    {
        auto pos1=2*index;
        ptr[index]=convert_func(data_ptr[pos1]);
        auto pos2=2*index+1;
        if(pos2>=len)break;
        ptr[index]=(ptr[index]<<4)|convert_func(data_ptr[pos2]);
    }
    return {buf,ret_len};
}

void CanUtilSocket::initCanProxy(std::string dev_nam,uint32_t baund_rate,uint32_t can_baund_rante)
{
    invoke(Object::getCurrentThreadId(),[=](){
        if(proxy){
            AIMY_WARNNING("can proxy is inited!");
        }
        else {
            proxy.reset(new CanSerialProxy(this,dev_nam,baund_rate,can_baund_rante));
            proxy->notifyRecvFrame.connectFunc([this](std::shared_ptr<can_frame> frame){
                handCanFrame(*frame);
            });
            proxy->initSerialProxy();
        }
    });
}

void CanUtilSocket::controlPacketUnitTest(CanDeviceAddr addr,uint8_t data1,uint8_t data2,uint32_t data_len)
{
    uint8_t data_send[2]={data1,data2};
    auto frame=packCanFrame(data_send,data_len,READ_CAN_DEVICE_GROUP_ADDR(APPLICATION_DEVICE_ADDR),READ_CAN_DEVICE_GROUP_ADDR(addr)
                                    ,physicalAddrToCanId(static_cast<CanPhysicalAddr>(READ_CAN_PHYSICAL_ADDR(addr))));
    AIMY_DEBUG("size %lu ",frame.data_len);
    for(auto i:frame.optionCanIdList)
    {
        AIMY_DEBUG("%08x ",i);
    }
    std::string data_print;
    for(uint32_t len=0;len<frame.data_len;++len)
    {
        if(!data_print.empty())data_print+=" ";
        char buf[3];
        sprintf(buf,"%02x",frame.data.get()[len]);
        data_print+=buf;
    }
    AIMY_DEBUG("data:%s",data_print.c_str());
}

void CanUtilSocket::sendCanFrame(const canControlFrame&frame)
{
    invoke(Object::getCurrentThreadId(),[=](){
        if(deviceStatus!=CanDeviceStart&&!proxy)return ;
        if(sendDataList.size()>MaxCanFrameCacheSize){
            AIMY_WARNNING("drop send can frames,cache limit [%d]",MaxCanFrameCacheSize);
            return;
        }
        sendDataList.push_back(frame);
        if(!proxy)
        {
            workChannel->enablWriting();
            workChannel->sync();
        }
        else {
            proxy->enableWrite();
        }
    });
}

void CanUtilSocket::on_error()
{
    AIMY_ERROR("can socket error[%s]",strerror(platform::getErrno()));
    if(deviceStatus!=CanDeviceStart)return;
    reconnect();
}

void CanUtilSocket::on_close()
{
    AIMY_ERROR("can socket close[%s]",strerror(platform::getErrno()));
    if(deviceStatus!=CanDeviceStart)return;
    reconnect();
}

void CanUtilSocket::on_recv()
{
    if(deviceStatus!=CanDeviceStart)return;
    const int frame_size=sizeof (can_frame);
    struct can_frame frame;
    int nbytes;
    nbytes = recv(workChannel->getFd(), &frame,frame_size, 0);
    if(nbytes<frame_size)
    {
        AIMY_WARNNING("recv error[%s]",strerror(platform::getErrno()));
    }
    else {
        ++recvFrameCounts;
        sendTimeoutTimer->stop();
        recvTimeoutTimer->stop();
        handCanFrame(frame);
        notifyFrameCountForTest.emit(recvFrameCounts,touchScreenFrames,boardFrames);
    }
}

void CanUtilSocket::on_write()
{
    if(deviceStatus!=CanDeviceStart)return ;
    for(auto iter=sendDataList.begin();iter!=sendDataList.end();)
    {
        uint32_t offset=0;
        auto idIter=iter->optionCanIdList.begin();
        for(;idIter!=iter->optionCanIdList.end();++idIter)
        {
            auto canId=*idIter;
            uint32_t send_offset=offset;
            uint32_t send_len=(send_offset+8<=iter->data_len)?8:(iter->data_len-send_offset);
            if(send_len==0){
                idIter=iter->optionCanIdList.end();
                break;
            }
            //send frame
            struct can_frame frame;
            memset(&frame,0,sizeof (frame));
            frame.can_id=canId|CAN_EFF_FLAG;
            frame.can_dlc=send_len;
            memcpy(frame.data,iter->data.get()+send_offset,send_len);
#ifdef DEBUG
            char buf[32];
            memset(buf,0,32);
            sprintf(buf,"%02X%02X%02X%02X",(canId>>24)&0xff,(canId>>16)&0xff,(canId>>8)&0xff,canId&0xff);
            buf[9]=0;
            int fill_offset=10;
            for(uint32_t i=0;i<send_len;++i)
            {
                sprintf(buf+fill_offset,"%02X",frame.data[i]);
                fill_offset+=2;
            }
            AIMY_DEBUG("can send to [%s][%s]",buf,buf+10);
#endif

            int nbytes = send(workChannel->getFd(), &frame, sizeof(struct can_frame), 0);
            if(nbytes<0)
            {
                auto error_num=platform::getErrno();
                AIMY_ERROR("can send failed [%s]",strerror(error_num));
                if(error_num!=EAGAIN&&error_num!=EINTR){
                    goto CAN_SEND_FAIL;
                }
                else {
                    break;
                }
            }
            offset+=send_len;
        }
        if(idIter==iter->optionCanIdList.end())
        {
            iter=sendDataList.erase(iter);
        }
        else {
            break;
        }
    }
    sendTimeoutTimer->start();
    if(sendDataList.empty())
    {
        workChannel->disableWriting();
        workChannel->sync();
    }
    return;
CAN_SEND_FAIL:
    reconnect();
}

void CanUtilSocket::on_reconnect()
{
    if(deviceStatus!=CanDeviceReconnecting)return;
    if(initCanSocket())
    {
        setDeviceState(CanDeviceStart);
    }
    else {
        reinitTimer->start();
    }
}

void CanUtilSocket::on_send_timeout()
{
    AIMY_WARNNING("can send timeout");
    notifyDeviceDisconnected.emit();
}

void CanUtilSocket::handCanFrame(const can_frame &frame)
{
    uint32_t can_id=frame.can_id;
    if (can_id & CAN_ERR_FLAG) {
        can_id &=(CAN_ERR_MASK|CAN_ERR_FLAG);
    } else if (can_id & CAN_EFF_FLAG) {
        can_id &=CAN_EFF_MASK;
    } else {
        can_id&=CAN_SFF_MASK;
    }
#ifdef DEBUG
    AIMY_BACKTRACE("recv %08X data[%u]->%s",can_id,frame.can_dlc,convertHexBuf(frame.data,frame.can_dlc).c_str());
#else
    AIMY_DEBUG("recv %08X data[%u]->%s",can_id,frame.can_dlc,convertHexBuf(frame.data,frame.can_dlc).c_str());
#endif
    auto frame_cache=getFrameCache(can_id);
    if(!frame_cache)
    {
        std::string message{"can recv cache is not init!"};
        AIMY_FATALERROR("%s",message.c_str());
        throw std::runtime_error(message);
        return;
    }
    if(frame_cache->data_len+frame.can_dlc>frame_cache->default_capacity)
    {
        AIMY_WARNNING("can frame overflow!discard it!");
        return;
    }
    memcpy(frame_cache->data.get()+frame_cache->data_len,frame.data,frame.can_dlc);
    frame_cache->data_len+=frame.can_dlc;
    do{
        auto protocal_frame=unpackControlFrame(frame_cache);
        if(protocal_frame.error_flag){
            notifyCanDeviceErroForTest.emit();
            break;
        }
        if(protocal_frame.data_len>0)
        {
            handleMessage(protocal_frame);
        }
        else {
            if(frame_cache->data_len>=8){
                if(frame_cache->data_len>=8)recvTimeoutTimer->start();
            }
            break;
        }
    }while(1);
}

void CanUtilSocket::setDeviceState(CanDeviceStatus status)
{
    if(status!=deviceStatus)
    {
        deviceStatus.exchange(status);
        notifyCanDeviceWorkSate(status);
    }
}

bool CanUtilSocket::initCanSocket()
{
    if(workChannel)
    {
        workChannel->stop();
        NETWORK_UTIL::close_socket(workChannel->getFd());
        workChannel.reset();
    }
    bool ret=false;
    SOCKET workFd=-1;
    do{
        workFd=socket(PF_CAN,SOCK_RAW,CAN_RAW);
        if(workFd==INVALID_SOCKET)
        {
            AIMY_ERROR("canfd init failed [%s]",strerror(platform::getErrno()));
            break;
        }
        struct ifreq ifr;
        memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
        strncpy(ifr.ifr_name, canDeviceName.c_str(), canDeviceName.length());
        if (ioctl(workFd, SIOCGIFINDEX, &ifr) < 0) {
            AIMY_ERROR("get %s index failed [%s]",canDeviceName.c_str(),strerror(platform::getErrno()));
            break;
        }

        addr.can_family=AF_CAN;
        addr.can_ifindex=ifr.ifr_ifindex;
        if (bind(workFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            AIMY_ERROR("bind can work fd failed [%s]",strerror(platform::getErrno()));
            break;
        }
        //允许接收自身发出的消息
        uint32_t recv_own_msgs=1;
        setsockopt(workFd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs));

        AIMY_DEBUG("init can socket success");
        workChannel=scheduler->addChannel(workFd);
        workChannel->bytesReady.connect(this,std::bind(&CanUtilSocket::on_recv,this));
        workChannel->errorEvent.connect(this,std::bind(&CanUtilSocket::on_error,this));
        workChannel->writeReady.connect(this,std::bind(&CanUtilSocket::on_write,this));
        workChannel->closeEvent.connect(this,std::bind(&CanUtilSocket::on_close,this));
        workChannel->enableReading();
        workChannel->sync();
        return true;
    }while(0);
    if(!ret)
    {
        NETWORK_UTIL::close_socket(workFd);
    }
    return ret;
}

void CanUtilSocket::reconnect()
{
    stopCan();
    setDeviceState(CanDeviceReconnecting);
    AIMY_ERROR("reconnect can after %ld msec",reinitTimer->getInterval());
    reinitTimer->start();
}

std::shared_ptr<canControlFrame> CanUtilSocket::getFrameCache(uint32_t canId)
{
    canId=physicalAddrToCanId(canIdToPhysicalAddr(canId));
    auto iter=canFrameCache.find(canId);
    if(iter!=canFrameCache.end())return iter->second;
    return nullptr;
}

void CanUtilSocket::initCanFrameCache()
{
    initCanFramCacheByAddr(CanPhysicalAddrGroupUnknown);
    initCanFramCacheByAddr(CanPhysicalAddrApplicationGroup);
    initCanFramCacheByAddr(CanPhysicalAddr1PHandGroup);
    initCanFramCacheByAddr(CanPhysicalAddr2PHandGroup);
    initCanFramCacheByAddr(CanPhysicalAddr1PBoardGroup);
    initCanFramCacheByAddr(CanPhysicalAddr2PBoardGroup);
    initCanFramCacheByAddr(CanPhysicalAddrFrontStageLightGroup);
    initCanFramCacheByAddr(CanPhysicalAddrBackStageLightGroup);
    initCanFramCacheByAddr(CanPhysicalAddrCentralGroup);
    initCanFramCacheByAddr(CanPhysicalAddrBroadcastGroup);
}

void CanUtilSocket::initCanFramCacheByAddr(CanPhysicalAddr addr)
{
    auto canId=physicalAddrToCanId(addr);
    canFrameCache.emplace(canId,std::make_shared<canControlFrame>(256,canId));
}

std::string CanUtilSocket::convertHexBuf(const uint8_t *data,uint32_t len)
{
    char buf[33]={0};
    memset(buf,0,33);
    uint32_t offset=0;
    for(uint32_t i=0;i<len&&offset<31;++i)
    {
        sprintf(buf+offset,"%02X",data[i]);
        offset+=2;
    }
    return std::string(buf,len*2);
}

void CanUtilSocket::handleMessage(const canControlFrame &data)
{
    if(data.optionCanIdList.empty()||data.data_len<ApplicationMessageHeaderLen+1)return;
    uint16_t send_addr=0;

    CanPhysicalAddr group=canIdToPhysicalAddr(data.optionCanIdList.front());
    //filter control frame
    switch (group) {
    case CanPhysicalAddrApplicationGroup:
        send_addr=data.data.get()[4]<<8;
        send_addr=send_addr|data.data.get()[5];
        handleApplicationMessage(data,send_addr);
        break;
    default:
        AIMY_WARNNING("recv %04x frame %s",group,convertHexBuf(data.data.get(),data.data_len).c_str());
    }
}

void CanUtilSocket::handleApplicationMessage(const canControlFrame &data,uint16_t sendAddr)
{
    AIMY_DEBUG("%s from [%04x] %s",__func__,sendAddr,convertHexBuf(data.data.get()+CAN_HEAD_SIZE,data.data_len-CAN_HEAD_SIZE).c_str());
    switch (sendAddr) {
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_1):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_3):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_4):{
        ++boardFrames;
        handle1PHandSensor(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_1):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_3):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_4):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_5):{
        ++boardFrames;
        handle1PFootSensor(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_1):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_3):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_4):{
        ++boardFrames;
        handle2PHandSensor(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_1):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_3):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_4):
    case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_5):{
        ++boardFrames;
        handle2PFootSensor(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(TOUCH_SCREEN_GROUP_ADDR):{
        ++touchScreenFrames;
        handleTouchScreenMessage(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(RGB_EFFECT_LIGHT_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(RGB_EFFECT_LIGHT_ADDR_6):{
        handleHardwareTest(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(KEYBOARD_GROUP_ADDR_2):
    case READ_CAN_DEVICE_GROUP_ADDR(KEYBOARD_GROUP_ADDR_1):{
        handleInertCoin(data,sendAddr);
        break;}
    case READ_CAN_DEVICE_GROUP_ADDR(KEYBOARD_GROUP_ADDR_6):{
        notifyBackgroundButtonPressed.emit();
        break;}
    default:
        AIMY_WARNNING("undefined can device %04X",sendAddr);
        break;
    }
}

void CanUtilSocket::handleTouchScreenMessage(const canControlFrame &data,uint16_t sendAddr)
{
    static const uint32_t check_data_len=5;
    if(data.data_len!=ApplicationMessageHeaderLen+check_data_len){
        AIMY_WARNNING("illegal touchscreen can frame len[%u %u]",ApplicationMessageHeaderLen+check_data_len,data.data_len);
        return;
    }

    auto data_ptr=data.data.get();
    uint16_t x=data_ptr[ApplicationMessageHeaderLen]|(data_ptr[ApplicationMessageHeaderLen+1]<<8);
    uint16_t y=data_ptr[ApplicationMessageHeaderLen+2]|(data_ptr[ApplicationMessageHeaderLen+3]<<8);
    uint8_t event=data_ptr[ApplicationMessageHeaderLen+4];
    AIMY_DEBUG("can touch screen event %hu,%hu event:%02x",x,y,event);
    switch (event) {
    case ScreenPressed:
    case ScreenHoldOn:
    case ScreenReleased:
        notifyScreenTouchEvent.emit(x,y,static_cast<ScreenTouchEvent>(event));
        break;
    default:
        AIMY_WARNNING("illegal touchscreen event %02x",event);
        break;
    }
}

void CanUtilSocket::handle1PHandSensor(const canControlFrame &data,uint16_t sendAddr)
{
    auto data_len=data.data_len-ApplicationMessageHeaderLen;
    if(data_len>2)
    {
        AIMY_WARNNING("illegal hand sensor can frame len[%u %u]",ApplicationMessageHeaderLen+2,data.data_len);
        return;
    }
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if (data_len==2) {
        if(!isBoardTestData(data_ptr[0]))
        {
            AIMY_WARNNING("test hand sensor can frame check failed %02x",data_ptr[0]);
            return;
        }
        auto state=data_ptr[1];
        std::list<uint8_t>maskList{0x04,0x10,0x40,0x80,0x20,0x08};
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_1):{
            std::list<uint8_t>indexList{0x01,0x02,0x03,0x04,0x05,0x06};//从左往右
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_4):{
            std::list<uint8_t>indexList{0x07,0x08,0x09,0x0A,0x0B,0x0C};//从下往上
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_2):{
            std::list<uint8_t>indexList{0x0D,0x0E,0x0F,0x10,0x11,0x12};//从右往左
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_3):{
            std::list<uint8_t>indexList{0x13,0x14,0x15,0x16,0x17,0x18};//从上往下
            handleSensorState(state,indexList,maskList);
            break;}
        default:
            break;
        }
    }
    else {
        auto state=data_ptr[0];
        if(!isValidBoardState(state)){
            AIMY_WARNNING("invalid hand sensor state %02x",state);
            return;
        }
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_1):
            notifyBoardFiledStatusChanged(HAND_1P_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_2):
            notifyBoardFiledStatusChanged(HAND_1P_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_3):
            notifyBoardFiledStatusChanged(HAND_1P_LEFT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_HAND_GROUP_ADDR_4):
            notifyBoardFiledStatusChanged(HAND_1P_RIGHT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        default:
            break;
        }
    }
}

void CanUtilSocket::handle2PHandSensor(const canControlFrame &data,uint16_t sendAddr)
{
    auto data_len=data.data_len-ApplicationMessageHeaderLen;
    if(data_len>2)
    {
        AIMY_WARNNING("illegal hand sensor can frame len[%u %u]",ApplicationMessageHeaderLen+2,data.data_len);
        return;
    }
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if (data_len==2) {
        if(!isBoardTestData(data_ptr[0]))
        {
            AIMY_WARNNING("test hand sensor can frame check failed %02x",data_ptr[0]);
            return;
        }
        auto state=data_ptr[1];
        std::list<uint8_t>maskList{0x04,0x10,0x40,0x80,0x20,0x08};
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_1):{
            std::list<uint8_t>indexList{0x41,0x42,0x43,0x44,0x45,0x46};//从左往右
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_4):{
            std::list<uint8_t>indexList{0x47,0x48,0x49,0x4A,0x4B,0x4C};// 从上往下
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_2):{
            std::list<uint8_t>indexList{0x4D,0x4E,0x4F,0x50,0x51,0x52};// 从右往左
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_3):{
            std::list<uint8_t>indexList{0x53,0x54,0x55,0x56,0x57,0x58};// 从下往上
            handleSensorState(state,indexList,maskList);
            break;}
        default:
            break;
        }
    }
    else {
        auto state=data_ptr[0];
        if(!isValidBoardState(state)){
            AIMY_WARNNING("invalid hand sensor state %02x",state);
            return;
        }
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_1):
            notifyBoardFiledStatusChanged(HAND_2P_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_2):
            notifyBoardFiledStatusChanged(HAND_2P_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_3):
            notifyBoardFiledStatusChanged(HAND_2P_LEFT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_HAND_GROUP_ADDR_4):
            notifyBoardFiledStatusChanged(HAND_2P_RIGHT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        default:
            break;
        }
    }
}

void CanUtilSocket::handle1PFootSensor(const canControlFrame &data,uint16_t sendAddr)
{
    auto data_len=data.data_len-ApplicationMessageHeaderLen;
    if(data_len>2)
    {
        AIMY_WARNNING("illegal foot sensor can frame len[%u %u]",ApplicationMessageHeaderLen+2,data.data_len);
        return;
    }
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if (data_len==2) {
        if(!isBoardTestData(data_ptr[0]))
        {
            if(sendAddr==READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_1)&&data_ptr[0]==0XE1)
            {
                notify1PGroupSuccess.emit();
                return;
            }
            else {
                AIMY_WARNNING("test foot sensor can frame check failed %02x",data_ptr[0]);
                return;
            }

        }
        auto state=data_ptr[1];

        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_1):{
            std::list<uint8_t>maskList{0x80,0x20,0x08,0x02,0x01,0x04,0x10,0x40};
            std::list<uint8_t>indexList{0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20};//1p左前
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_2):{
            std::list<uint8_t>maskList{0x10,0x40,0x80,0x20,0x08,0x02,0x01,0x04};
            std::list<uint8_t>indexList{0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28};//1p右前
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_3):{
            std::list<uint8_t>maskList{0x08,0x02,0x01,0x04,0x10,0x40,0x80,0x20};
            std::list<uint8_t>indexList{0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};//1p左后
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_4):{
            std::list<uint8_t>maskList{0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08};
            std::list<uint8_t>indexList{0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x40};//1p右后
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_5):{
            std::list<uint8_t>maskList{0x40,0x80,0x20,0x08,0x02,0x01,0x04,0x10};
            std::list<uint8_t>indexList{0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30};//1p中
            handleSensorState(state,indexList,maskList);
            break;}
        default:
            break;
        }
    }
    else {
        auto state=data_ptr[0];
        if(!isValidBoardState(state)){
            AIMY_WARNNING("invalid foot sensor state %02x",state);
            return;
        }
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_1):
            notifyBoardFiledStatusChanged(FOOT_1P_LEFT_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_2):
            notifyBoardFiledStatusChanged(FOOT_1P_RIGHT_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_3):
            notifyBoardFiledStatusChanged(FOOT_1P_LEFT_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_4):
            notifyBoardFiledStatusChanged(FOOT_1P_RIGHT_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_1P_FOOT_GROUP_ADDR_5):
            notifyBoardFiledStatusChanged(FOOT_1P_MID_FILED,static_cast<BoardFiledStatus>(state));
            break;
        default:
            break;
        }
    }
}

void CanUtilSocket::handle2PFootSensor(const canControlFrame &data,uint16_t sendAddr)
{
    auto data_len=data.data_len-ApplicationMessageHeaderLen;
    if(data_len>2)
    {
        AIMY_WARNNING("illegal foot sensor can frame len[%u %u]",ApplicationMessageHeaderLen+2,data.data_len);
        return;
    }
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if (data_len==2) {
        if(!isBoardTestData(data_ptr[0]))
        {
            if(sendAddr==READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_1)&&data_ptr[0]==0XE1)
            {
                notify2PGroupSuccess.emit();
                return;
            }
            else {
                AIMY_WARNNING("test foot sensor can frame check failed %02x",data_ptr[0]);
                return;
            }
        }
        auto state=data_ptr[1];

        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_1):{
            std::list<uint8_t>maskList{0x80,0x20,0x08,0x02,0x01,0x04,0x10,0x40};
            std::list<uint8_t>indexList{0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60};//2p左前
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_2):{
            std::list<uint8_t>maskList{0x10,0x40,0x80,0x20,0x08,0x02,0x01,0x04};
            std::list<uint8_t>indexList{0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68};//2p右前
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_3):{
            std::list<uint8_t>maskList{0x08,0x02,0x01,0x04,0x10,0x40,0x80,0x20};
            std::list<uint8_t>indexList{0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78};//2p左后
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_4):{
            std::list<uint8_t>maskList{0x04,0x02,0x01,0x80,0x40,0x20,0x10,0x08};
            std::list<uint8_t>indexList{0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0x80};//2p右后
            handleSensorState(state,indexList,maskList);
            break;}
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_5):{
            std::list<uint8_t>maskList{0x40,0x80,0x20,0x08,0x02,0x01,0x04,0x10};
            std::list<uint8_t>indexList{0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70};//2p中
            handleSensorState(state,indexList,maskList);
            break;}
        default:
            break;
        }
    }
    else {
        auto state=data_ptr[0];
        if(!isValidBoardState(state)){
            AIMY_WARNNING("invalid foot sensor state %02x",state);
            return;
        }
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_1):
            notifyBoardFiledStatusChanged(FOOT_2P_LEFT_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_2):
            notifyBoardFiledStatusChanged(FOOT_2P_RIGHT_FRONT_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_3):
            notifyBoardFiledStatusChanged(FOOT_2P_LEFT_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_4):
            notifyBoardFiledStatusChanged(FOOT_2P_RIGHT_BACK_FILED,static_cast<BoardFiledStatus>(state));
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(SENSOR_2P_FOOT_GROUP_ADDR_5):
            notifyBoardFiledStatusChanged(FOOT_2P_MID_FILED,static_cast<BoardFiledStatus>(state));
            break;
        default:
            break;
        }
    }
}

bool CanUtilSocket::isBoardTestData(uint8_t byte)
{
    return byte==0x80;
}

void CanUtilSocket::handleSensorState(uint8_t byte,const std::list<uint8_t>&indexList,const std::list<uint8_t>&maskList)
{
    auto iter=indexList.begin();
    auto maskIter=maskList.begin();
    for(;iter!=indexList.end()&&maskIter!=maskList.end();++iter,++maskIter)
    {
        if(byte&*maskIter)notifyTestSensorStatusChanged(*iter,true);
        else {
            notifyTestSensorStatusChanged(*iter,false);
        }
    }
}

void CanUtilSocket::handleHardwareTest(const canControlFrame &data,uint16_t sendAddr)
{
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if(data_ptr[0]==0xE1) {
        switch (sendAddr) {
        case READ_CAN_DEVICE_GROUP_ADDR(RGB_EFFECT_LIGHT_ADDR_2):
            notifyFrontStageGroupSuccess.emit();
            break;
        case READ_CAN_DEVICE_GROUP_ADDR(RGB_EFFECT_LIGHT_ADDR_6):
            notifyBackStageGroupSuccess.emit();
            break;
        }
    }
    else {
        AIMY_WARNNING("invalid can frame check  value %02x",data_ptr[0]);
    }
}

void CanUtilSocket:: handleInertCoin(const canControlFrame &data,uint16_t /*sendAddr*/)
{
    auto data_ptr=data.data.get()+ApplicationMessageHeaderLen;
    if(data_ptr[0]==0x01)
    {
        notifyInsertCoin.emit();
    }
    else if(data_ptr[0]==0xE1) {
        notifyCentralControlSuccess.emit();
    }
    else {
        AIMY_WARNNING("invalid keyboard key value %02x",data_ptr[0]);
    }
}
bool CanUtilSocket::isValidBoardState(uint8_t byte)
{
    return byte==BoardFiledStatus::BoardPressed||byte==BoardFiledStatus::BoardHoldOn||byte==BoardFiledStatus::BoardReleased;
}
