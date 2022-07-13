#include "can-serial-proxy.h"
#include "can-util-socket.h"
using namespace aimy;
#ifdef DEBUG
std::atomic<uint64_t>can_recv_cnt(0);
std::atomic<uint64_t>can_send_cnt(0);
#endif
CanSerialProxy::CanSerialProxy(CanUtilSocket *parent,std::string dev_nam,uint32_t baund_rate,uint32_t can_baund_rante):Object(parent),notifyRecvFrame(this),canSocketSrc(parent),proxySerialName(dev_nam),serialBaundRate(baund_rate),canBaundRate(can_baund_rante)
{
    reinitTimer=parent->addTimer(5000);
    reinitTimer->setSingle(true);
    reinitTimer->timeout.connect(this,std::bind(&CanSerialProxy::initSerialProxy,this));
}

CanSerialProxy::~CanSerialProxy()
{
    reinitTimer->release();
}

void CanSerialProxy::init_statistics_state(bool send_state,bool recv_state)
{
    send_state_statistics=send_state;
    recv_state_statistics=recv_state;
}

void CanSerialProxy::initSerialProxy()
{
    invoke(Object::getCurrentThreadId(),[=](){
        AIMY_WARNNING("try init serial can proxy %s",proxySerialName.c_str());
        do{
            reinitTimer->stop();
            if(workChannel)
            {
                workChannel->stop();
                NETWORK_UTIL::close_socket(workChannel->getFd());
            }
            workChannel.reset();
            auto handle=aserial_open(proxySerialName.c_str());
            if(handle<=0)
            {
                AIMY_ERROR("can't open %s[%s]",proxySerialName.c_str(),strerror(errno));
                break;
            }
            aserial_set_opt(handle,serialBaundRate,8,'N',1);
            aserial_set_rts(handle,true);
            workChannel=canSocketSrc->scheduler->addChannel(handle);
            workChannel->bytesReady.connect(this,std::bind(&CanSerialProxy::on_recv,this));
            workChannel->errorEvent.connect(this,std::bind(&CanSerialProxy::on_error,this));
            workChannel->writeReady.connect(this,std::bind(&CanSerialProxy::on_write,this));
            workChannel->closeEvent.connect(this,std::bind(&CanSerialProxy::on_close,this));
            workChannel->enableReading();
            workChannel->sync();
            initDataList.clear();
            initDataList.push_back(std::string("AT+AT\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+USART_PARAM=?\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+USART_PARAM=1500000,0,0,0\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+CAN_MODE=?\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+CAN_MODE=0\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+CAN_BAUD=?\r\n"));
            initDataList.push_back(std::string("AT+CG\r\n"));
            initDataList.push_back(std::string("AT+CAN_BAUD=")+std::to_string(canBaundRate)+"\r\n");
            initDataList.push_back(std::string("AT+AT\r\n"));
            enableWrite();
            AIMY_WARNNING("init serial can proxy %s success",proxySerialName.c_str());
            return ;
        }while(0);
        reinitTimer->start();
    });
}

void CanSerialProxy::enableWrite()
{
    AIMY_DEBUG("proxy enable write %p",this);
    if(workChannel)
    {
        workChannel->enablWriting();
        workChannel->sync();
    }
}

void CanSerialProxy::on_error()
{
    reinitTimer->start();
}

void CanSerialProxy::on_close()
{
    reinitTimer->start();
}

void CanSerialProxy::on_recv()
{
    const uint32_t cache_size=1024;
    uint8_t buf[cache_size]={0};
    memset(buf,0,cache_size);
    auto ret=read(workChannel->getFd(),buf,cache_size);
    if(ret>0)
    {
#ifndef DEBUG
        char printf_buf[cache_size*3+1]={0};
        char *ptr=printf_buf;
        memset(printf_buf,0,cache_size*3+1);
        int print_offset=0;
        for(int i=0;i<ret;++i)
        {
            print_offset+=snprintf(ptr+print_offset,4,"%02X ",buf[i]);
        }
        AIMY_DEBUG("serial recv by[%s] [%s]->[%s]",proxySerialName.c_str(),std::string(reinterpret_cast<char *>(buf),ret).c_str(),printf_buf);
#endif
        if(ret<7||buf[0]!=0x41||buf[1]!=0x54)return;
        uint32_t data_len=ret;
        uint8_t *data_ptr=buf;
        // 2 AT + 4 can_id + 1 len + 0d 0a
        for(uint32_t i=0;i<=data_len-9;)
        {
            //find A
            while (i<=data_len-9) {
                while(i<data_len&&data_ptr[i]!=0x41)
                {
                    ++i;
                }
                ++i;
                if(i<=data_len-8&&data_ptr[i]==0x54)
                {
                    ++i;
                    break;
                }
            }
            //4 can_id + 1 len + 2 "\r\n"
            if(data_len-i<7)break;
            //parser can_id
            uint32_t tmp=0;
            tmp=((data_ptr[i+0])<<24)|(data_ptr[i+1]<<16)|(data_ptr[i+2]<<8)|(data_ptr[i+3]);
            uint32_t can_id=tmp>>3;
            //read payload
            uint8_t payload_len=data_ptr[i+4];
            //check
            //4 can_id + 1 len + payload + 2 "\r\n"
            if(data_len-i<(4+1+payload_len+2)||data_ptr[i+4+1+payload_len]!=0x0d||data_ptr[i+4+1+payload_len+1]!=0x0a)break;
            uint8_t *payload_start=data_ptr+i+5;
            can_frame frame;
            frame.can_id=can_id|CAN_EFF_FLAG;
            frame.can_dlc=payload_len;
            memcpy(frame.data,payload_start,payload_len);
            canSocketSrc->handCanFrame(frame);
#ifdef DEBUG
            if(recv_state_statistics)
            {
                can_recv_cnt++;
                AIMY_BACKTRACE("serial recv by[%s] [send:%llu recv:%llu][%08x]->[%s]",proxySerialName.c_str(),can_send_cnt.load(),can_recv_cnt.load(),can_id,aimy::AimyLogger::formatHexToString(payload_start,payload_len,false).c_str());
            }
#endif
            i+=5+payload_len+2;//move to next frame
        }
    }
}

void CanSerialProxy::on_write()
{
    if(!initDataList.empty())
    {
        auto data=initDataList.front();

        auto ret=write(workChannel->getFd(),data.c_str(),data.size());
        if(ret<0||static_cast<uint32_t>(ret)!=data.size())
        {
            AIMY_WARNNING("send failed %s",strerror(errno));
        }
        else
        {
            AIMY_INFO("serial send %d [%s]",ret,data.c_str());
            initDataList.pop_front();
            usleep(40000);
        }
    }
    else if(!canSocketSrc->sendDataList.empty()){
        auto &sendDataList=canSocketSrc->sendDataList;
        const uint32_t max_cache_len=4+5+8;
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

                uint8_t send_cache[max_cache_len];
                //encode
                send_cache[0]=0x41;//A
                send_cache[1]=0x54;//T
                send_cache[2]=0x00;
                send_cache[3]=0x00;
                send_cache[4]=0x00;
                send_cache[5]=0x04;
                send_cache[6]=0x00;

                uint32_t tmp=(canId<<3)|0x04;
                send_cache[2]=(tmp>>24)&0xff;
                send_cache[3]=(tmp>>16)&0xff;
                send_cache[4]=(tmp>>8)&0xff;
                send_cache[5]=tmp&0xff;
                send_cache[6]=send_len;

                memcpy(send_cache+7,iter->data.get()+send_offset,send_len);

                send_cache[7+send_len]=0x0d;//\r
                send_cache[8+send_len]=0x0a;//\n
                AIMY_INFO("serial send %08x [%s]",canId,AimyLogger::formatHexToString(send_cache,max_cache_len+send_len-8).c_str());
                int nbytes =write(workChannel->getFd(),send_cache,max_cache_len+send_len-8);
                if(nbytes<0)
                {
                    AIMY_WARNNING("serial send failed [%s]",strerror(errno));
                    break;
                }
                else {
#ifdef DEBUG
                    if(send_state_statistics)
                    {
                        can_send_cnt++;
                        AIMY_ERROR("serial send->cnt[%llu] %d [%s]",can_send_cnt.load(),nbytes,AimyLogger::formatHexToString(send_cache,max_cache_len+send_len-8).c_str());
                    }
#else
                    AIMY_INFO("serial send %d [%s]",nbytes,AimyLogger::formatHexToString(send_cache,max_cache_len+send_len-8).c_str());
#endif
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
    }
    if(initDataList.empty()&&canSocketSrc->sendDataList.empty())
    {
        workChannel->disableWriting();
        workChannel->sync();
    }
}
