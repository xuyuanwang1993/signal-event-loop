#include "websocket_api_base.h"
using namespace aimy;
WebsocketApiBase::WebsocketApiBase(const std::string &api_name,bool is_passive,TaskScheduler *_parent,SOCKET _fd):TcpConnection(_parent,_fd)
  ,protocalWebsocket(nullptr),isPassive(is_passive),apiName(api_name)
{
    protocalWebsocket.reset(new aimy::ProtocalWebsocket());
    setProtocal(protocalWebsocket);
}

WebsocketApiBase::~WebsocketApiBase()
{

}

void WebsocketApiBase::setProtocal(std::shared_ptr<ProtocalBase>_protocal)
{
    TcpConnection::setProtocal(_protocal);
}

bool WebsocketApiBase::sendFrame(const void *frame, uint32_t frame_len)
{
    if(frame_len==0)return true;

    auto ret=writeCache->appendFrame(frame,frame_len);
    if(writeCache->frame_count()==1)
    {
        channel->enablWriting();
        channel->sync();
    }
    return ret>0&&ret==frame_len;
}

void WebsocketApiBase::on_recv()
{
    while(1)
    {
        auto ret=readCache->readFromFd();
        if(ret<0)break;
        else if (ret==0) {
            AIMY_DEBUG("%s recv remote maybe disconnected,close it!",description().c_str());
            on_close();
            break;
        }
        //
        while(1)
        {
            auto frame=readCache->popFrame();
            if(frame.second>0)
            {
                StreamWebsocketFrame w_frame;
                if(protocalWebsocket->bufferToFrame(frame.first.get(),frame.second,&w_frame))
                {
                    if(w_frame.header.MASK)
                    {
                        for(uint32_t i=0;i<w_frame.header.actual_payload_len;++i)
                        {
                            w_frame.payload[i]^=w_frame.header.Masking_key[i&0x3];
                        }
                    }
                    handleWebsocketFrame(w_frame);
                }
                else {
                    AIMY_WARNNING("%s maybe something wrong?",description().c_str());
                }
            }else {
                break;
            }
        }
    }
}

void WebsocketApiBase::on_write()
{
    while(writeCache->frame_count()>0)
    {
        auto ret=writeCache->sendCacheByFd();
        if(ret<0)break;
        if(ret==0)
        {
            AIMY_DEBUG("%s write remote maybe disconnected,close it!",description().c_str());
            on_close();
            return;
        }
    }
    if(writeCache->frame_count()==0)
    {
        channel->disableWriting();
        channel->sync();
    }
}

void WebsocketApiBase::on_close()
{
    AIMY_ERROR("%s closed,disconnect it! [%s]",description().c_str(),strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();
}

void WebsocketApiBase::on_error()
{
    AIMY_ERROR("%S error,disconnect it! [%s]",description().c_str(),strerror(NETWORK_UTIL::get_socket_error(fd)));
    channel->stop();
    disconnected();
}

bool WebsocketApiBase::sendWebsocketFrame(WS_FrameType type,const void *payload,uint32_t payload_len)
{
    std::shared_ptr<uint8_t>frame_copy(new uint8_t[payload_len+1],std::default_delete<uint8_t[]>());
    memset(frame_copy.get(),0,payload_len+1);
    memcpy(frame_copy.get(),payload,payload_len);
    return invoke(Object::getCurrentThreadId(),[=](){
        auto frame=protocalWebsocket->packetFrame(frame_copy.get(),payload_len,!isPassive,type,true);
        if(!sendFrame(frame.first.get(),frame.second))
        {
            AIMY_ERROR("%s send failed!",description().c_str());
            on_close();
            return false;
        }
        return true;
    });

}

void WebsocketApiBase::handleWebsocketFrame(const StreamWebsocketFrame &frame)
{
    switch (frame.header.opcode) {
    case WS_TEXT_FRAME:
        handleTextFrame(frame);
        break;
    case WS_CONNECTION_CLOSE_FRAME:
        handleCloseFrame(frame);
        break;
    case WS_PING_FRAME:
        handlePingFrame(frame);
        break;
    case WS_PONG_FRAME:
        handlePongFrame(frame);
        break;
    default:
        handleOtherFrame(frame);
        break;
    }
}

void WebsocketApiBase::handleTextFrame(const StreamWebsocketFrame &frame)
{
    if(frame.header.actual_payload_len>0)
    {
        uint32_t debug_len=frame.header.actual_payload_len>1000?1000:frame.header.actual_payload_len;
        AIMY_DEBUG("%s recv text frame [%ld]\n[%s]",description().c_str(),frame.header.actual_payload_len,std::string( reinterpret_cast<const char *>( frame.payload),debug_len).c_str());
    }
    else {
        AIMY_DEBUG("%s recv text frame [%ld]",description().c_str(),frame.header.actual_payload_len);
    }
    //just echo
    sendWebsocketFrame(WS_TEXT_FRAME,frame.payload,frame.header.actual_payload_len);
}

void WebsocketApiBase::handlePongFrame(const StreamWebsocketFrame &frame)
{
    if(frame.header.actual_payload_len>0)
    {
        uint32_t debug_len=frame.header.actual_payload_len>1000?1000:frame.header.actual_payload_len;
        AIMY_DEBUG("%s recv pong frame [%ld] [%s]",description().c_str(),frame.header.actual_payload_len,AimyLogger::formatHexToString(frame.payload,debug_len).c_str());
    }
    else {
        AIMY_DEBUG("%s recv pong frame [%ld]",description().c_str(),frame.header.actual_payload_len);
    }
}

void WebsocketApiBase::handlePingFrame(const StreamWebsocketFrame &frame)
{
    if(frame.header.actual_payload_len>0)
    {
        uint32_t debug_len=frame.header.actual_payload_len>1000?1000:frame.header.actual_payload_len;
        AIMY_DEBUG("%s recv ping frame [%ld] [%s]",description().c_str(),frame.header.actual_payload_len,AimyLogger::formatHexToString(frame.payload,debug_len).c_str());
    }
    else {
        AIMY_DEBUG("%s recv ping frame [%ld]",description().c_str(),frame.header.actual_payload_len);
    }
    sendWebsocketFrame(WS_PONG_FRAME,frame.payload,frame.header.actual_payload_len);
}

void WebsocketApiBase::handleCloseFrame(const StreamWebsocketFrame &frame)
{
    (void)frame;
    AIMY_WARNNING("%s active disconnected!",description().c_str());
    on_close();
}

void WebsocketApiBase::handleOtherFrame(const StreamWebsocketFrame &frame)
{
    if(frame.header.actual_payload_len>0)
    {
        uint32_t debug_len=frame.header.actual_payload_len>1000?1000:frame.header.actual_payload_len;
        AIMY_DEBUG("%s recv [%u %ld] [%s]",description().c_str(),frame.header.opcode,frame.header.actual_payload_len,AimyLogger::formatHexToString(frame.payload,debug_len).c_str());
    }
    else {
        AIMY_DEBUG("%s recv [%u %ld]",description().c_str(),frame.header.opcode,frame.header.actual_payload_len);
    }

}