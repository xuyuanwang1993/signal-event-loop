#include "protocal_vd_stream.h"
#include "core/core-include.h"
#include<cstring>
using namespace aimy::virtual_device;
ProtocalStream::ProtocalStream(uint32_t _max_cache_size, uint32_t _max_frame_size, uint32_t _slice_size):ProtocalBase(_max_cache_size,true,_max_frame_size,_slice_size==0?1200:_slice_size)
{

}

ProtocalStream::~ProtocalStream()
{

}

std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalStream::packetFrame(const void *header, uint16_t header_len, const void * payload, uint32_t payload_len)
{
    uint32_t length=platform::localToBe32(minProtocalSize()+header_len+payload_len);
    uint16_t version=0;
    uint16_t compress_flag=0;
    uint32_t stream_offset=platform::localToBe32(header_len);
    if(payload_len==0)
    {
        stream_offset=~0;
    }
    std::list<std::pair<const void *,uint32_t>> input_list;
    input_list.push_back(std::make_pair(&length,4));
    input_list.push_back(std::make_pair(&version,2));
    input_list.push_back(std::make_pair(&compress_flag,2));
    input_list.push_back(std::make_pair(&stream_offset,4));
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
        if(input_list.size()<6)
        {
            AIMY_ERROR("false input for Pframe encode");
            break;
        }
        if(total_len>getMaxFrameSize())break;
        ret_len=total_len;
        ret_buf.reset(new uint8_t[total_len],std::default_delete<uint8_t[]>());
        memset(ret_buf.get(),0,total_len);
        auto iter=input_list.begin();
        uint32_t offset=0;
        //length
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //version
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //compress_flag
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //stream_offset
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //header
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //payload
        ++iter;
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
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
        auto need_size=header.length;
        if(header.version!=0||need_size>getMaxFrameSize())
        {
            AIMY_ERROR("header check failed!");
            frame_len=getMaxFrameSize()+len;
            break;
        }
        if(need_size>len)break;
        frame_len=need_size;
    }while(0);
    return {frame_start,frame_len};
}

StreamFrameHeader ProtocalStream::decodeFrameHeader(const uint8_t *data)
{
    StreamFrameHeader header;
    header.length=platform::beToLocal32(*(reinterpret_cast<const uint32_t *>(data)));
    header.version=platform::beToLocal16(*(reinterpret_cast<const uint16_t *>(data+4)));
    header.compress_flag=platform::beToLocal16(*(reinterpret_cast<const uint16_t *>(data+6)));
    header.stream_offset=platform::beToLocal32(*(reinterpret_cast<const uint32_t *>(data+8)));
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
        auto size=frame->header.length;
        if(size!=buf_len)
        {
            AIMY_ERROR("%u != %u,size not match",size,buf_len);
            break;
        }
        frame->payload_header=ptr+minProtocalSize();
        frame->payload=ptr+minProtocalSize()+frame->header.getHeaderLen();
        ret=true;

    }while(0);
    return ret;

}
