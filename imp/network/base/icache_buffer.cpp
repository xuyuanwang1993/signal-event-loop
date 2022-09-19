#include "icache_buffer.h"
#include "protocal-base.h"
using namespace aimy;
IcacheBufferBase::IcacheBufferBase(std::shared_ptr<ProtocalBase> _protocal, SOCKET _fd, uint32_t _max_cache_size):
    protocal(_protocal),fd(_fd),max_cache_size(_max_cache_size<_protocal->getMaxCacheSize()?protocal->getMaxCacheSize():_max_cache_size),cache_buf(nullptr)
  ,cache_size(0),write_offset(0),decode_offset(0),read_buffer(nullptr),read_buffer_size(0)
{

}

IcacheBufferBase::~IcacheBufferBase()
{

}

ssize_t IcacheBufferBase::readFromFd ()
{
    if(!read_buffer)
    {
        read_buffer_size=protocal->getSliceSize();
        read_buffer.reset(new uint8_t[read_buffer_size+1],std::default_delete<uint8_t[]>());
        memset(read_buffer.get(),0,read_buffer_size+1);
        if(!read_buffer)
        {
            AIMY_ERROR("malloc buffer failed!");
            return -1;
        }
    }
    auto ret=::recv(fd,read_buffer.get(),read_buffer_size,0);
    if(ret<0)
    {
        int err=platform::getErrno();
        if(err!=EAGAIN&&err!=EINTR)
        {
            return 0;
        }
    }
    if(ret>0)
    {//recv a frame
        if(readFromBuf(read_buffer.get(),ret)<0)return 0;
    }
    return ret;
}

ssize_t IcacheBufferBase::readFromBuf(const void *data,uint32_t buf_len)
{
    if(!appendCache(data,buf_len))
    {
        AIMY_ERROR("apeend cache failed!");
        return -1;
    }
    while(1)
    {
        auto ret=parseCache();
        if(ret.second==0||ret.second>protocal->getMaxFrameSize())break;
        updateFrameCache(ret.first,ret.second);
    }
    return buf_len;
}

std::pair<std::shared_ptr<uint8_t>,uint32_t> IcacheBufferBase::popFrame()
{
    std::shared_ptr<uint8_t> ret_buf;
    uint32_t ret_len=0;
    do{
        if(frame_cache_map.empty())break;
        auto frame_info=frame_cache_map.front();
        ret_len=frame_info.second;
        if(ret_len>0)
        {
            ret_buf.reset(new uint8_t[ret_len+1],std::default_delete<uint8_t[]>());
            memset(ret_buf.get(),0,ret_len+1);
            memcpy(ret_buf.get(),cache_buf.get()+frame_info.first,ret_len);
        }
        frame_cache_map.pop_front();
    }while(0);
    return {ret_buf,ret_len};
}

ssize_t IcacheBufferBase::appendRawData(const void *data,uint32_t len)
{
    std::list<std::pair<const void *,uint32_t>> input_list;
    input_list.push_back({data,len});
    auto frame=protocal->encodeFrame(input_list);
    return appendFrame(frame.first.get(),frame.second);
}

ssize_t IcacheBufferBase::appendFrame(const void *data,uint32_t len)
{
    if(!appendCache(data,len))
    {
        AIMY_ERROR("apeend cache failed!");
        return -1;
    }
    updateFrameCache(write_offset-len,len);
    return len;
}

void IcacheBufferBase::resize(uint32_t new_size)
{
    auto old_buf=cache_buf;
    cache_buf.reset(new uint8_t[new_size+1],std::default_delete<uint8_t[]>());
    memset(cache_buf.get(),0,new_size+1);
    auto read_offset=decode_offset;
    if(!frame_cache_map.empty())read_offset=frame_cache_map.front().first;
    auto data_size=write_offset-read_offset;
    //update offset
    for(auto &i:frame_cache_map)
    {
        i.first-=read_offset;
    }
    decode_offset-=read_offset;
    write_offset-=read_offset;
    //copy data
    cache_size=new_size;
    memcpy(cache_buf.get(),old_buf.get()+read_offset,data_size);
    old_buf.reset();
}

bool IcacheBufferBase::appendCache(const void *data,uint32_t len)
{
    if(len==0)return false;
    if(len+write_offset>cache_size)
    {
        //        此处有两种情况，
        //        1.数据可用空间不足
        //        2.缓存低地址存在可用的空间
        uint32_t read_offset=decode_offset;
        if(!frame_cache_map.empty())read_offset=frame_cache_map.front().first;
        uint32_t data_size=write_offset-read_offset;
        if(data_size+len>max_cache_size){
            AIMY_ERROR("buffer overflow!max_size[%u] -> need [%u]",max_cache_size,data_size+len);
            return false;
        }
        uint32_t available_size=cache_size-(write_offset-read_offset);
        uint32_t new_size=0;
        //case 1
        if(available_size>10*len)
        {//尝试缩小所用内存
            new_size=data_size+protocal->getMaxFrameSize();
        }//case 2
        else {
            new_size=cache_size+protocal->getMaxFrameSize();
        }
        if(new_size>max_cache_size)new_size=max_cache_size;
        resize(new_size);
    }
    memcpy(cache_buf.get()+write_offset,data,len);
    write_offset+=len;
    return true;
}

ssize_t IcacheBufferBase::sendCacheByFd()
{//try to send a frame
    if(frame_cache_map.size()==0)return -1;
    int64_t ret=0;
    auto frame=frame_cache_map.front();
    while(frame.second>0)
    {
        auto send_size=frame.second>protocal->getSliceSize()?protocal->getSliceSize():frame.second;
        auto send_ret=::send(fd,cache_buf.get()+frame.first,send_size,0);
        if(send_ret<0)
        {
            int err=platform::getErrno();
            if(err!=EAGAIN&&err!=EINTR)
            {
                return 0;
            }
            break;
        }
        if(frame.second!=send_ret&&!protocal->isStreaming())return 0;
        frame.first+=send_ret;
        frame.second-=send_ret;
        ret+=send_ret;
    }
    if(frame.second==0)frame_cache_map.pop_front();
    if(ret==0)ret=-1;
    return ret;
}

std::pair<uint32_t,uint32_t> IcacheBufferBase::parseCache()
{
    auto ret=protocal->decodeFrame(static_cast<const uint8_t *>(cache_buf.get())+decode_offset,write_offset-decode_offset);
    if(ret.second>protocal->getMaxFrameSize())
    {
        AIMY_WARNNING("invalid data!");
        decode_offset+=ret.second-protocal->getMaxFrameSize();
    }
    //recaculate offset
    ret.first+=decode_offset;
    return ret;
}

void IcacheBufferBase::updateFrameCache(uint32_t frame_offset, uint32_t frame_len)
{
    frame_cache_map.push_back(std::make_pair(frame_offset,frame_len));
    //update decode offset
    decode_offset=frame_offset+frame_len;
}
