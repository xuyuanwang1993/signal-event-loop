#include "protocal-base.h"
#include <string.h>
using namespace aimy;
ProtocalBase::ProtocalBase(uint32_t _max_cache_size,bool _is_streaming,uint32_t _max_frame_size,uint32_t _slice_size):max_cache_size(_max_cache_size),
    is_streaming(_is_streaming),max_frame_size(_max_frame_size),slice_size(_slice_size==0?4096:_slice_size)
{

}

ProtocalBase::~ProtocalBase()
{

}

std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalBase::encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list)
{
    std::shared_ptr<uint8_t>ret_buf;
    uint32_t ret_len=0;
    uint32_t total_len=0;
    for(const auto &i:input_list)
    {
        total_len+=i.second;
    }
    do{
        if(total_len>getMaxFrameSize())break;
        ret_len=total_len;
        ret_buf.reset(new uint8_t[total_len],std::default_delete<uint8_t[]>());
        memset(ret_buf.get(),0,total_len);
        uint32_t offset=0;
        for(const auto &i:input_list)
        {
            memcpy(ret_buf.get()+offset,i.first,i.second);
            offset+=i.second;
        }
    }while(0);
    return {ret_buf,ret_len};
}

std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> ProtocalBase::decodeFrame(const void *data,uint32_t len)
{
    (void)data;
    return {0,len};
}
