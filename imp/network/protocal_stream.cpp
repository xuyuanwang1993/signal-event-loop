#include "protocal_stream.h"
#include "core/core-include.h"
#include<cstring>
using namespace aimy;
#define PROTOCAL_STREAM_VERSION 1
#define PROTOCAL_STREAM_MAX_TIME_DIFF 1000*5*60   //3min
ProtocalStream::ProtocalStream(uint32_t _max_cache_size, uint32_t _max_frame_size, uint32_t _slice_size):ProtocalBase(_max_cache_size,true,_max_frame_size,_slice_size==0?1200:_slice_size)
{

}

ProtocalStream::~ProtocalStream()
{

}

std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalStream::packetFrame(const void *header, uint16_t header_len, const void * payload, uint32_t payload_len)
{
    static const uint8_t version=PROTOCAL_STREAM_VERSION;
    uint8_t check_code=caculateCheckcode(header,header_len,payload,payload_len);
    uint16_t header_length=platform::localToBe16(header_len);
    uint32_t payload_length=platform::localToBe32(payload_len);
    uint64_t timestamp=platform::localToBe64(Timer::getTimeNow<std::chrono::milliseconds>());

    std::list<std::pair<const void *,uint32_t>> input_list;
    input_list.push_back(std::make_pair(&version,1));
    input_list.push_back(std::make_pair(&check_code,1));
    input_list.push_back(std::make_pair(&header_length,2));
    input_list.push_back(std::make_pair(&payload_length,4));
    input_list.push_back(std::make_pair(&timestamp,8));
    input_list.push_back(std::make_pair(header,header_len));
    input_list.push_back(std::make_pair(payload,payload_len));
    return encodeFrame(input_list);
}


std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalStream::encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list)
{
    std::shared_ptr<uint8_t>ret_buf;
    uint32_t ret_len=0;
    uint32_t total_len=0;
    for(const auto &i:input_list)
    {
        total_len+=i.second;
    }
    do{
        if(input_list.size()<7)
        {
            AIMY_ERROR("false input for Pframe encode");
            break;
        }
        if(total_len>getMaxFrameSize())break;
        ret_len=total_len;
        ret_buf.reset(new uint8_t[total_len+1],std::default_delete<uint8_t[]>());
        memset(ret_buf.get(),0,total_len+1);
        uint32_t offset=0;
        auto iter=input_list.begin();
        uint32_t timestamp_offset=0;
        uint32_t timestamp_len=0;
        uint32_t data_len=0;
        //version
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //check_code
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //header_length
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //payload_length
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //timestamp
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        timestamp_offset=offset;
        timestamp_len=iter->second;
        offset+=iter->second;
        //header
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        data_len+=iter->second;
        offset+=iter->second;
        //payload
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        data_len+=iter->second;

        encryptPayload(ret_buf.get()+timestamp_offset,timestamp_len,ret_buf.get()+timestamp_offset+timestamp_len,data_len);
    }while(0);
    return {ret_buf,ret_len};
}

std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> ProtocalStream::decodeFrame(const void *data,uint32_t len)
{
    uint32_t frame_start=0;
    uint32_t frame_len=0;
    do{
        if(len<minProtocalSize())break;
        auto header=decodeFrameHeader(reinterpret_cast<const uint8_t *>(data));
        auto need_size=minProtocalSize()+header.header_length+header.payload_length;
        uint64_t time_now=Timer::getTimeNow<std::chrono::milliseconds>();
        if(header.version!=PROTOCAL_STREAM_VERSION||header.reserved!=0||need_size>getMaxFrameSize()
                ||(header.timestamp<time_now&&(time_now-header.timestamp)>PROTOCAL_STREAM_MAX_TIME_DIFF))
        {
            AIMY_DEBUG("version:%d %d",header.version,PROTOCAL_STREAM_VERSION);
            AIMY_DEBUG("reserved:%d",header.reserved);
            AIMY_DEBUG("need_size %u:%u",need_size,getMaxFrameSize());
            AIMY_DEBUG("time %lu:%lu:%lu:%lu",header.timestamp,time_now,(time_now-header.timestamp),PROTOCAL_STREAM_MAX_TIME_DIFF);
            AIMY_ERROR("header check failed!");
            frame_len=getMaxFrameSize()+len;
            break;
        }
        if(need_size>len)break;
        frame_len=need_size;
    }while(0);
    return {frame_start,frame_len};
}

uint8_t ProtocalStream::caculateCheckcode(const void *header,uint32_t header_len,const void * payload,uint32_t payload_len)
{
    uint8_t sum=0;
    const uint8_t * ptr=reinterpret_cast<const uint8_t *>(header);
    for(uint32_t i=0;i<header_len;++i){
        sum+=ptr[i];
    }
    ptr=reinterpret_cast<const uint8_t *>(payload);
    for(uint32_t i=0;i<payload_len;++i){
        sum+=ptr[i];
    }
    return 0x100-sum;
}

void ProtocalStream::encryptPayload(const void *mask,uint32_t mask_len,void *data,uint32_t data_len)
{
    if(mask_len==0)return ;
    const uint8_t * mask_map=reinterpret_cast<const uint8_t *>(mask);
    uint8_t *data_ptr=reinterpret_cast<uint8_t *>(data);
    for(uint32_t offset=0;offset<data_len;++offset)
    {
        data_ptr[offset]^=mask_map[offset%mask_len];
    }
}

StreamFrameHeader ProtocalStream::decodeFrameHeader(const uint8_t *data)
{
    StreamFrameHeader header;
    header.version=data[0]&0xf;
    header.reserved=data[0]>>4;
    header.check_code=data[1];
    header.header_length=platform::beToLocal16(*(reinterpret_cast<const uint16_t *>(data+2)));
    header.payload_length=platform::beToLocal32(*(reinterpret_cast<const uint32_t *>(data+4)));
    header.timestamp=platform::beToLocal64(*(reinterpret_cast<const uint64_t *>(data+8)));
    return header;
}

bool ProtocalStream::bufferToFrame(void *buf, uint32_t buf_len, StreamFrame * frame)
{
#ifdef DEBUG
    assert(frame!=nullptr);
#endif
    bool ret=false;
    auto ptr=reinterpret_cast<uint8_t *>(buf);
    do{
        if(buf_len<minProtocalSize())break;
        frame->header=decodeFrameHeader(ptr);
        auto size=minProtocalSize()+frame->header.header_length+frame->header.payload_length;
        if(size!=buf_len)
        {
            AIMY_ERROR("%u != %u,size not match",size,buf_len);
            break;
        }
        frame->payload_header=ptr+minProtocalSize();
        frame->payload=ptr+minProtocalSize()+frame->header.header_length;
        uint64_t decode_mask=platform::localToBe64(frame->header.timestamp);
        encryptPayload(&decode_mask,sizeof (frame->header.timestamp),frame->payload_header,frame->header.payload_length+frame->header.header_length);
        auto check_code=caculateCheckcode(frame->payload_header,frame->header.header_length,frame->payload,frame->header.payload_length);
        if(check_code!=frame->header.check_code)
        {
            AIMY_ERROR("%u != %u,check_code not match",check_code,frame->header.check_code);
            break;
        }
        ret=true;

    }while(0);
    return ret;

}
