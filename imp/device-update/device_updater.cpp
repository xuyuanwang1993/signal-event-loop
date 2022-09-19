#include "device_updater.h"
#include "common/hid_utils.h"
#include "common/serial_utils.h"
#include "Protocol.h"
#include "common/iutils.h"
using namespace aimy;
AimyUpdater::AimyUpdater(TaskScheduler *parent):Object(parent),notifyQuitMessage(this),notifyQueryMessage(this),notifyUpdateMessage(this),notifyUpdateProgress(this),
    scheduler(parent),workChannel(nullptr),messageUrl(""),usingSerial(false),updateDeviceAdress(0),quitStatus(UPGRADE_ERROR_WITHOUT_ROLLBACK),quitMessage("normal"),dataLen(0),sliceContext(nullptr),lastUpdateFrame(nullptr),lastUpdateFrameLen(0),resendTimer(nullptr),totalPackets(0),progressPackets(0)
{
    memset(readCache,0,4096);
    resendTimer=parent->addTimer(5000);
    resendTimer->setSingle(true);
    resendTimer->timeout.connect(this,std::bind(&AimyUpdater::on_send_timeout,this));
}

AimyUpdater::~AimyUpdater()
{
    release();
    resendTimer->release();
    resendTimer.reset();
}

bool AimyUpdater::init(const std::string &url)
{
    if(!url.empty())
    {
        release();
        messageUrl=url;
        SOCKET fd=-1;
        if(strstr(url.c_str(),"hid:"))
        {//hid_device
            uint32_t vid=0;
            uint32_t pid=0;
            if(sscanf(url.c_str(),"hid:%04x:%04x",&vid,&pid)!=2)
            {
                AIMY_ERROR("invalid hid url format %s,need match hid:%%04x:%%04x!",url.c_str());
                return false;
            }
            auto dev_list= HidDevice::hidFind(pid,vid);
            if(dev_list.empty())
            {
                AIMY_ERROR("can't find hid %s!",url.c_str());
                return false;
            }

            usingSerial=false;
            auto dev_path=dev_list.front();
            fd=HidDevice::open(dev_path);
            if(fd<0)
            {
                AIMY_ERROR("open %s failed!",dev_path.c_str());
                return false;
            }
            setenv("FORCE_UPDATE","true",1);
        }
        else {
            //use tty
            auto handle=aserial_open(url.c_str());
            if(!handle)
            {
                AIMY_ERROR("open %s failed!",url.c_str());
                return false;
            }
            aserial_set_opt(handle,460800,8,'N',1);
            fd=handle;
            usingSerial=true;
        }
        AIMY_DEBUG("open %s success [%d]",url.c_str(),fd);
        workChannel=scheduler->addChannel(fd);
        workChannel->bytesReady.connect(this,std::bind(&AimyUpdater::on_read,this));
        workChannel->errorEvent.connect(this,std::bind(&AimyUpdater::on_error,this));
        workChannel->writeReady.connect(this,std::bind(&AimyUpdater::on_write,this));
        workChannel->closeEvent.connect(this,std::bind(&AimyUpdater::on_close,this));
        workChannel->enableReading();
        workChannel->sync();
        return true;
    }
    else {
        AIMY_ERROR("path is empty!");
        return false;
    }
}

bool AimyUpdater::loadUpdateTask(const std::string &file_path, uint32_t device_adress)
{
    updateDeviceAdress=device_adress;
    return context.init(file_path);
}

void AimyUpdater::startUpdate()
{
    invoke(Object::getCurrentThreadId(),[this](){
        requestEnterUpgradeWorkMode();
    });
}

void AimyUpdater::stopUpdate()
{
    invoke(Object::getCurrentThreadId(),[this](){
        requestQuitUpgradeWorkMode("active quit");
    });
}

void AimyUpdater::queryDevice()
{
    invoke(Object::getCurrentThreadId(),[this](){
        requestQueryDevice();
    });
}

void AimyUpdater::queryUPID()
{
    invoke(Object::getCurrentThreadId(),[this](){
        requestQueryUPID();
    });
}

void AimyUpdater::queryWorkMode()
{
    invoke(Object::getCurrentThreadId(),[this](){
        requestQueryWorkMode();
    });
}



void AimyUpdater::handleDataCache()
{
#ifdef DEBUG
    AIMY_BACKTRACE("recv data cache size[%u] data->[%s]",dataLen,AimyLogger::formatHexToString(readCache,dataLen).c_str());
#endif
    uint8_t *p_frame;
    uint32_t frame_length;
    unsigned address;
    unsigned control_code;
    size_t i = 0;
    for (; i < dataLen; ++i) {
        auto err = Protocol::DeMux(readCache + i, dataLen - i, (void**)&p_frame, frame_length, &address, control_code);
        switch (err) {
        case 0:
        {
            Protocol::ControlCode code;
            code.byte = (decltype(code.byte))control_code;
            handleFrame(p_frame,frame_length,address,code.bit.control_frame_switch);
            dataLen=0;
            return;
        }
        case 1:
            return;
        default:

            break;
        }
    }

    dataLen = 0;
}

void AimyUpdater::handleFrame(const void *data,uint32_t data_len,uint32_t address,uint32_t control_code)
{
    if(control_code==0)
    {
        AIMY_WARNNING("this frame maybe protocal frame,discard it!");
        return;
    }
#ifdef DEBUG
    AIMY_DEBUG("recv size [%u] frame ->%s",data_len,AimyLogger::formatHexToString(data,data_len).c_str());
#endif
    uint8_t command = ((const uint8_t*)data)[0];
    const uint8_t *p_data=static_cast<const uint8_t *>(data);
    --data_len;
    p_data++;
    switch (command) {
    case QUERY_DEVICE:
        handleReplyQueryDevice(p_data,data_len,address);
        notifyQueryMessage(QUERY_DEVICE,"query_device");
        break;
    case QUERY_UPID:
        handleReplyQueryUPID(p_data,data_len,address);
        notifyQueryMessage(QUERY_DEVICE,"query_upid");
        break;
    case TRANSFER_ERROR:
    {
        // need retransfer?
        uint8_t error=p_data[0];
        AIMY_ERROR("transfer error %02x",error);
    }
        notifyQueryMessage(TRANSFER_ERROR,"TRANSFER_ERROR");
        break;
    case QUERY_WORK_MODE:
        handleReplyQueryWorkMode(p_data,data_len,address);
        notifyQueryMessage(QUERY_DEVICE,"query_workmode");
        break;
    // update
    case UPGRADE_ENTER:
        handleReplyEnterUpgradeWorkMode(p_data,data_len,address);
        notifyUpdateMessage(UPGRADE_ENTER);
        break;
    case UPGRADE_QUERY_SOFTWARE:
        handleReplyQuerySoftwareVersion(p_data,data_len,address);
        notifyUpdateMessage(UPGRADE_QUERY_SOFTWARE);
        break;
    case UPGRADE_QUERY_SEGMENT:
        handleReplyQuerySegmentInfomation(p_data,data_len,address);
        notifyUpdateMessage(UPGRADE_QUERY_SEGMENT);
        break;
    case UPGRADE_INT_VECTOR_TABLE:
        handleReplyUploadVectorTable(p_data,data_len,address);
        notifyUpdateMessage(UPGRADE_INT_VECTOR_TABLE);
        break;
    case UPGRADE_CODE_SEGMENT:
        handleReplyUploadCodeSegment(p_data,data_len,address);
        notifyUpdateMessage(UPGRADE_CODE_SEGMENT);
        break;
    case UPGRADE_RESUME:
        AIMY_ERROR("not supported!resume");
        break;
    case UPGRADE_QUIT:
        handleReplyQuitUpgradeWorkMode(p_data,data_len,address);
        break;
    default:
        AIMY_ERROR("unknown command %02x",command);
        break;
    }
}

void AimyUpdater::requestQueryDevice()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    UpdateTypeDef command=QUERY_DEVICE;
    sendNormalData(&command,sizeof (command));
}

void AimyUpdater::requestQueryUPID()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    UpdateTypeDef command=QUERY_UPID;
    sendNormalData(&command,sizeof (command));
}

void AimyUpdater::requestQueryWorkMode()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    UpdateTypeDef command=QUERY_WORK_MODE;
    sendNormalData(&command,sizeof (command));
}

void AimyUpdater::requestEnterUpgradeWorkMode()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    size_t max_size_frame = 64;//hid max frame size
    max_size_frame -= Protocol::GetMinProtocolSize() +1+2+2;//command
    max_size_frame &= 0xFC;
    sliceContext.reset(new UpdateSliceContext(max_size_frame));
    UpdateTypeDef command=UPGRADE_ENTER;
    sendUpdateData(&command,sizeof (command));
}

void AimyUpdater::requestQuerySoftwareVersion()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    UpdateTypeDef command=UPGRADE_QUERY_SOFTWARE;
    sendUpdateData(&command,sizeof (command));
}

void AimyUpdater::requestQuerySegmentInfomation()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s",__FUNCTION__);
#endif
    UpdateTypeDef command=UPGRADE_QUERY_SEGMENT;
    sendUpdateData(&command,sizeof (command));
}

void AimyUpdater::requestUploadSegment()
{
#ifdef DEBUG
    AIMY_BACKTRACE("%s --%lu %lu %d",__FUNCTION__,sliceContext->vector_map.size(),sliceContext->code_map.size(),sliceContext->seq);
#endif
#pragma pack(push, 1)
    struct {
        uint8_t command;
        uint16_t packet_no;
        uint16_t crc;
        uint8_t data[64];
    } content;
#pragma pack(pop)
    memset(&content,0,sizeof (content));
    content.command=sliceContext->type;
    content.packet_no= htobe16(sliceContext->seq&0xffff);
    memcpy(content.data,sliceContext->data.get(),sliceContext->data_len);
    content.crc=htobe16(Iutils::crc_rc16(content.data,sliceContext->data_len));
    sendUpdateData(&content,1+2+2+sliceContext->data_len);
}


void AimyUpdater::requestQuitUpgradeWorkMode(const std::string &message)
{
    AIMY_BACKTRACE("%s->%s",__FUNCTION__,message.c_str());
    quitMessage=message;
#pragma pack(push, 1)
    struct {
        UpdateTypeDef command;
        UpgradeQuitStatus status;
    } content;
#pragma pack(pop)

    content.command = UPGRADE_QUIT;
    content.status = quitStatus;
    sendNormalData(&content,sizeof (content));
}

void AimyUpdater::handleReplyQueryDevice(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv query device reply from [%08x]",address);
    const char *str_version , *str_product, *str_date;
    if(data_len>1&&parserDeviceInfo(data,data_len,&str_version,&str_product,&str_date))
    {
        AIMY_INFO("query device result product[%s] version[%s] date[%s]",str_product,str_version,str_date);
    }
    else {
        AIMY_WARNNING("query device result with false format");
    }
}

void AimyUpdater::handleReplyQueryUPID(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv query upid reply from [%08x]",address);
    if(data_len>1)
    {
        struct {
            uint8_t upid;
        } *p_reply = (decltype(p_reply))data;
        AIMY_INFO("query upid result upid[%02x]",p_reply->upid);
    }
    else {
        AIMY_WARNNING("query upid result with false format");
    }

}

void AimyUpdater::handleReplyQueryWorkMode(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv query workmode reply from [%08x] ",address);
    if(data_len>1)
    {
#pragma pack(push, 1)
        struct {
            uint8_t workmode;
            uint8_t upid;
        } *p_reply = (decltype(p_reply))data;
#pragma pack(pop)
        AIMY_INFO("query workmode result upid[%02x] workmode[%02x]",p_reply->upid,p_reply->workmode);
    }
    else {
        AIMY_WARNNING("query workmode result with false format");
    }
}

void AimyUpdater::handleReplyEnterUpgradeWorkMode(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv enter upgrade mode reply from [%08x]",address);
    if(address!=updateDeviceAdress)return;
#pragma pack (push, 1)
    struct ReplyUpgradeEnter {
        enum : uint8_t {
            BUSY = 0,	//未就绪
            FULL = 1,	//完整升级就绪
            RESUME = 2,	//中断续传升级就绪
            REPORT = 3	//设备主动报告升级就绪
        } status;
        uint8_t page_size;	//设备flash页尺寸,以k为单位
        uint8_t chip_id;//芯片ID
    };
#pragma pack(pop)
    const ReplyUpgradeEnter *p_reply = (decltype(p_reply))data;
    if(data_len<sizeof (ReplyUpgradeEnter))
    {
        requestQuitUpgradeWorkMode("enter upgrade reply false format");
        return;
    }
    sliceContext->page_size=p_reply->page_size*1024;
    AIMY_DEBUG("upgrade status %02x page_size %dk chip_id %02x",p_reply->status,p_reply->page_size,p_reply->chip_id);
    if(p_reply->status==ReplyUpgradeEnter::FULL||p_reply->status==ReplyUpgradeEnter::RESUME||p_reply->status==ReplyUpgradeEnter::REPORT)
    {
        AIMY_DEBUG("update status [%02x]",p_reply->status);
        requestQuerySoftwareVersion();
    }
    else {
        requestQuitUpgradeWorkMode("device is busy or not support!");
        return;

    }
}

void AimyUpdater::handleReplyQuerySoftwareVersion(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv  query software version reply from [%08x]",address);
    if(address!=updateDeviceAdress)return;
#pragma pack (push, 1)
    struct ReplyUpgradeQuerySoftware {
        uint32_t  version_addr;
        uint32_t  product_addr;
        uint32_t  date_addr;
        char info[0];//三个字符串,以'\0'分隔
    };
#pragma pack(pop)
    if(data_len<sizeof (ReplyUpgradeQuerySoftware))
    {
        requestQuitUpgradeWorkMode("upgrade query software upgrade reply false format");
        return;
    }
    const ReplyUpgradeQuerySoftware *p_reply = (const ReplyUpgradeQuerySoftware*)data;
    const char *str_version, *str_product, *str_date;
    if(!parserDeviceInfo(p_reply->info,data_len-3*sizeof (uint32_t),&str_version,&str_product,&str_date))
    {
        requestQuitUpgradeWorkMode("upgrade query software upgrade reply parse device info failed!");
        return;
    }
    //
    auto ret=context.checkFile(str_version,str_product,str_date,p_reply->version_addr,p_reply->date_addr,p_reply->date_addr);
    if(ret==0)
    {
        requestQuitUpgradeWorkMode("not need upgrade!");
        return;
    }
    else if (ret<0) {
        quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
        requestQuitUpgradeWorkMode("file check failed!");
        return;
    }
    else {
        requestQuerySegmentInfomation();
    }
}

void AimyUpdater::handleReplyQuerySegmentInfomation(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv  query segment info reply from [%08x]",address);
    if(address!=updateDeviceAdress)return;
#pragma pack (push, 1)
    struct ReplyUpgradeQuerySegment {
        uint32_t int_vector_address;
        uint32_t code_segment_address;
        uint32_t int_vector_end_address;
        uint32_t code_segment_end_address;
    };
#pragma pack(pop)
    if(data_len<sizeof (ReplyUpgradeQuerySegment))
    {
        requestQuitUpgradeWorkMode("upgrade query segment info reply false format");
        return;
    }
    const ReplyUpgradeQuerySegment *p_reply = (const ReplyUpgradeQuerySegment*)data;
    if(!context.loadProgramData(be32toh(p_reply->int_vector_address),be32toh(p_reply->int_vector_end_address),be32toh(p_reply->code_segment_address)
                                ,be32toh(p_reply->code_segment_end_address)))
    {
        quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
        requestQuitUpgradeWorkMode("file load failed!");
        return;
    }
    if(!context.fillUpdateSliceContext(*sliceContext.get())||sliceContext->data_len==0)
    {
        quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
        requestQuitUpgradeWorkMode("read slice failed!");
        return;
    }
    totalPackets=sliceContext->code_map.size()+sliceContext->vector_map.size();
    AIMY_ERROR("totalPackets:%u %u %u",totalPackets,sliceContext->vector_map.size(),sliceContext->code_map.size());
    progressPackets=0;
    requestUploadSegment();
}

void AimyUpdater::handleReplyUploadVectorTable(const void *data,uint32_t data_len,uint32_t address)
{
#ifdef DEBUG
    AIMY_DEBUG("recv upload vector table reply from [%08x]",address);
#endif
    if(address!=updateDeviceAdress)return;
#pragma pack (push, 1)
    struct ReplyUpgradeUploadSegment
    {
        enum : uint8_t {
            SUCCESS = 0,			//接收成功
            MISSING = 0x54,			//丢包
            FLASH_ERASE = 0x55,		//擦除FLASH失败(需要退出升级,提示硬件损坏)
            FLASH_WRITE = 0x56,		//写FLASH失败(需要退出升级,提示硬件损坏);
            DATA_ADDRESS = 0x57,	//数据地址错误(不在升级地址范围内)
            DEVICE_ADDRESS = 0x58,	//升级设备的设备地址错误
        } status;
        uint16_t reply_packet_no;	//应答包号
        uint16_t last_packet_no;	//设备最后确认的包号, status=MISSING时存在
    };
    const ReplyUpgradeUploadSegment *p_reply = (const ReplyUpgradeUploadSegment*)data;
#pragma pack(pop)
    if (data_len < sizeof(ReplyUpgradeUploadSegment) - sizeof(ReplyUpgradeUploadSegment::last_packet_no)
            ||	(p_reply->status == ReplyUpgradeUploadSegment::MISSING && data_len < sizeof(ReplyUpgradeUploadSegment))) {
        quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
        requestQuitUpgradeWorkMode("upgrade upload vector table reply false format!");
        return;
    }
    uint16_t reply_packet_no = be16toh(p_reply->reply_packet_no);
    if(reply_packet_no!=(sliceContext->seq&0xffff))
    {
        //AIMY_WARNNING("repeat packet reply[%04x] now[%04x]",reply_packet_no,sliceContext->seq&0xffff);
        return;
    }
    switch (p_reply->status) {
    case ReplyUpgradeUploadSegment::SUCCESS:
        updateProgress();
        sliceContext->seq--;
        if(!context.fillUpdateSliceContext(*sliceContext.get())||sliceContext->data_len==0)
        {
            quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
            requestQuitUpgradeWorkMode("read slice failed!");
            return;
        }
        requestUploadSegment();
        break;
    case ReplyUpgradeUploadSegment::MISSING:
        on_send_timeout();
        break;
    default:
        AIMY_ERROR("upgrade:status",p_reply->status);
        break;
    }
}

void AimyUpdater::handleReplyUploadCodeSegment(const void *data,uint32_t data_len,uint32_t address)
{
#ifdef DEBUG
    AIMY_DEBUG("recv upload code segment reply from [%08x]",address);
#endif
    if(address!=updateDeviceAdress)return;
#pragma pack (push, 1)
    struct ReplyUpgradeUploadSegment
    {
        enum : uint8_t {
            SUCCESS = 0,			//接收成功
            MISSING = 0x54,			//丢包
            FLASH_ERASE = 0x55,		//擦除FLASH失败(需要退出升级,提示硬件损坏)
            FLASH_WRITE = 0x56,		//写FLASH失败(需要退出升级,提示硬件损坏);
            DATA_ADDRESS = 0x57,	//数据地址错误(不在升级地址范围内)
            DEVICE_ADDRESS = 0x58,	//升级设备的设备地址错误
        } status;
        uint16_t reply_packet_no;	//应答包号
        uint16_t last_packet_no;	//设备最后确认的包号, status=MISSING时存在
    };
    const ReplyUpgradeUploadSegment *p_reply = (const ReplyUpgradeUploadSegment*)data;
#pragma pack(pop)
    if (data_len < sizeof(ReplyUpgradeUploadSegment) - sizeof(ReplyUpgradeUploadSegment::last_packet_no)
            ||	(p_reply->status == ReplyUpgradeUploadSegment::MISSING && data_len < sizeof(ReplyUpgradeUploadSegment))) {
        quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
        requestQuitUpgradeWorkMode("upgrade upload code  segment reply false format!");
        return;
    }
    uint16_t reply_packet_no = be16toh(p_reply->reply_packet_no);
    if(reply_packet_no!=(sliceContext->seq&0xffff))
    {
        //AIMY_WARNNING("repeat packet reply[%04x] now[%04x]",reply_packet_no,sliceContext->seq&0xffff);
        return;
    }
    switch (p_reply->status) {
    case ReplyUpgradeUploadSegment::SUCCESS:
        updateProgress();
        if(sliceContext->seq>0)
        {
            sliceContext->seq--;
            if(!context.fillUpdateSliceContext(*sliceContext.get())||sliceContext->data_len==0)
            {
                AIMY_ERROR("%lu--------\r\n",sliceContext->data_len);
                quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
                requestQuitUpgradeWorkMode("read slice failed!");
                return;
            }
            requestUploadSegment();
        }
        else {
            quitStatus=UPGRADE_SUCCESS;
            requestQuitUpgradeWorkMode("success");
        }
        break;
    case ReplyUpgradeUploadSegment::MISSING:
        on_send_timeout();
        break;
    default:
        AIMY_ERROR("upgrade:status",p_reply->status);
        break;
    }
}

void AimyUpdater::handleReplyQuitUpgradeWorkMode(const void *data,uint32_t data_len,uint32_t address)
{
    AIMY_DEBUG("recv quit upgrade workmode reply from [%08x]",address);
    if(address!=updateDeviceAdress)return;
    resendTimer->stop();
    uint8_t status;
    if(data_len<1)
    {
        AIMY_ERROR("error quit frame");
    }
    else {
        status=*((const uint8_t *)data);
        if(status!=0&&quitStatus==UPGRADE_SUCCESS)
        {
            quitStatus=UPGRADE_ERROR_WITH_ROLLBACK;
            quitMessage="quit failed";
        }
    }
    notifyQuitMessage(quitStatus,quitMessage);
}
#ifdef HID_ENCRYPT
    static const uint8_t scg_key[] = {
        0xFF, 0x83, 0xDF, 0x17, 0x32, 0x09, 0x4E, 0xD1,
        0xE7, 0xCD, 0x8A, 0x91, 0xC6, 0xD5, 0xC4, 0xC4,
        0x40, 0x21, 0x18, 0x4E, 0x55, 0x86, 0xF4, 0xDC,
        0x8A, 0x15, 0xA7, 0xEC, 0x92, 0xDF, 0x93, 0x53,
        0x30, 0x18, 0xCA, 0x34, 0xBF, 0xA2, 0xC7, 0x59,
        0x67, 0x8F, 0xBA, 0x0D, 0x6D, 0xD8, 0x2D, 0x7D,
        0x54, 0x0A, 0x57, 0x97, 0x70, 0x39, 0xD2, 0x7A,
        0xEA, 0x24, 0x33, 0x85, 0xED, 0x9A, 0x1D, 0xE0
    };
#endif//HID_ENCRYPT
void AimyUpdater::on_read()
{
    uint32_t max_len=128;
    auto result=::read(workChannel->getFd(),readCache+dataLen,max_len);
    if(result<0)
    {
        int err=errno;
        switch (err){
        case EAGAIN:
        case EINTR:
            break;
        default:
            AIMY_ERROR("read %d failed [%s]",workChannel->getFd(),strerror(err));
            reload_upgrade_task();
        }
    }
    else {
#ifdef HID_ENCRYPT
    if(!usingSerial)
    {
        size_t *p_result = (size_t*)readCache+dataLen;
        size_t *p_key = (size_t*)scg_key;
        size_t enc_len = (result + sizeof(result) - 1) / sizeof(result);
        for (size_t i = 0; i < enc_len; ++i)
            p_result[i] ^= p_key[i];
    }
#endif//HID_ENCRYPT
        dataLen+=result;
        handleDataCache();
    }
}

void AimyUpdater::on_write()
{
    if(!sendDataList.empty())
    {
        while(!sendDataList.empty())
        {
            auto front=sendDataList.front();
            auto ret=write(workChannel->getFd(),front.first.get(),front.second);
            if(ret<0)
            {
                auto error=platform::getErrno();
                if(error!=EAGAIN&&error!=EINTR)
                {
                    reload_upgrade_task();
                    return;
                }
            }
            else if(ret==0){
                quitMessage="connection reset by remote peer";
                release();
                notifyQuitMessage(quitStatus,quitMessage);
                return;
            }
            else {
                AIMY_DEBUG("update send frame size %u data [%s]",ret,AimyLogger::formatHexToString(front.first.get(),front.second).c_str());
                sendDataList.pop_front();
            }

        }
    }
    else {
        if(lastUpdateFrame.get()&&!resendTimer->working())
        {
            auto ret=write(workChannel->getFd(),lastUpdateFrame.get(),lastUpdateFrameLen);
            if(ret<0)
            {
                auto error=platform::getErrno();
                if(error!=EAGAIN&&error!=EINTR)
                {
                    reload_upgrade_task();
                    return;
                }
            }
            else if(ret==0){
                quitMessage="connection reset by remote peer";
                release();
                notifyQuitMessage(quitStatus,quitMessage);
                return;
            }
            else {
                AIMY_DEBUG("update send frame size %u data [%s]",ret,AimyLogger::formatHexToString(lastUpdateFrame.get(),lastUpdateFrameLen).c_str());
                resendTimer->start();
            }
        }
    }
    if((resendTimer->working()||!lastUpdateFrame.get())&&sendDataList.empty())
    {
        workChannel->disableWriting();
        workChannel->sync();
    }
}

void AimyUpdater::on_close()
{
    AIMY_ERROR("AimyUpdater close");
    reload_upgrade_task();
}

void AimyUpdater::on_error()
{
    AIMY_ERROR("AimyUpdater error");
    reload_upgrade_task();
}

void AimyUpdater::on_send_timeout()
{
    resendTimer->stop();
    if(!workChannel)
    {
        return;
    }
    workChannel->enablWriting();
    workChannel->sync();
}

void AimyUpdater::release()
{
    if(workChannel)
    {
        workChannel->stop();
        NETWORK_UTIL::close_socket(workChannel->getFd());
        sendDataList.clear();
        lastUpdateFrame.reset();
        lastUpdateFrameLen=0;
    }
    workChannel.reset();
}

void AimyUpdater::reload_upgrade_task()
{
    if(updateDeviceAdress!=0)
    {
        int cnt=10;
        bool success=false;
        while(cnt-->0)
        {
            if(init(messageUrl))
            {
                startUpdate();
                success=true;
                break;
            }
            sleep(1);
        }
        if(!success)
        {
            quitMessage="connection disconnected";
            release();
            notifyQuitMessage(quitStatus,quitMessage);
        }
    }
    else {
        release();
    }
}

void AimyUpdater::sendNormalData(const void *data,uint32_t data_len)
{
    Protocol::ControlCode control_code;
    control_code.bit.control_frame_switch = 1;
    control_code.bit.read_write_flag = 0;

    auto cache=Iutils::allocBuffer(128);
    uint32_t ret_len=0;
    Protocol::Mux(data,data_len,nullptr,control_code.byte,cache.get(),ret_len);
#ifdef HID_ENCRYPT
    if(!usingSerial)
    {
        size_t *p_result = (size_t*)cache.get();
        size_t *p_key = (size_t*)scg_key;
        size_t enc_len = (ret_len + sizeof(ret_len) - 1) / sizeof(ret_len);
        for (size_t i = 0; i < enc_len; ++i)
            p_result[i] ^= p_key[i];
    }
#endif//HID_ENCRYPT
    sendDataList.push_back(std::make_pair(cache,ret_len));
    workChannel->enablWriting();
    workChannel->sync();
}

void AimyUpdater::sendUpdateData(const void *data,uint32_t data_len)
{
    resendTimer->stop();
    Protocol::ControlCode control_code;
    control_code.bit.control_frame_switch = 1;
    control_code.bit.read_write_flag = 1;

    auto cache=Iutils::allocBuffer(128);
    uint32_t ret_len=0;
    Protocol::Mux(data,data_len,&updateDeviceAdress,control_code.byte,cache.get(),ret_len);
#ifdef HID_ENCRYPT
    if(!usingSerial)
    {
        size_t *p_result = (size_t*)cache.get();
        size_t *p_key = (size_t*)scg_key;
        size_t enc_len = (ret_len + sizeof(ret_len) - 1) / sizeof(ret_len);
        for (size_t i = 0; i < enc_len; ++i)
            p_result[i] ^= p_key[i];
    }
#endif//HID_ENCRYPT
    lastUpdateFrame=cache;
    lastUpdateFrameLen=ret_len;
    workChannel->enablWriting();
    workChannel->sync();
}

void AimyUpdater::updateProgress()
{
    progressPackets++;
    if(progressPackets>totalPackets)progressPackets=totalPackets;
    if(totalPackets>0&&((progressPackets%100==0)||progressPackets==totalPackets))
    {
        notifyUpdateProgress(progressPackets*100.0/totalPackets);
    }
}

bool AimyUpdater::parserDeviceInfo(const void *data,uint32_t date_len,const char **p_version,const char ** p_product,const char **p_date)
{
    const char *p_info = (char*)data;
        const char **p_field[] = { p_version, p_product, p_date };
        int cur_field = 0;
        size_t cur_field_length = 0;
        for (size_t i = 0; i < date_len; ++i) {
            if (p_info[i] == '\0') {
                if (cur_field_length == 0)
                    return false;
                cur_field_length = 0;
            }
            else {
                ++cur_field_length;
                if (cur_field_length == 1) {
                    *p_field[cur_field++] = p_info + i;
                    if (cur_field == sizeof(p_field) / sizeof(*p_field))
                        return true;
                }
            }
        }
        return false;
}
