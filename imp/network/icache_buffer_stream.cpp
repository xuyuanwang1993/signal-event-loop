#include "icache_buffer_stream.h"

using namespace  aimy;
IcacheBufferStream::IcacheBufferStream(std::shared_ptr<ProtocalBase> _protocal, SOCKET _fd):IcacheBufferBase(_protocal,_fd,_protocal->getMaxCacheSize())
{

}

IcacheBufferStream::~IcacheBufferStream ()
{

}

ssize_t IcacheBufferStream::readFromFd ()
{
    if(!read_buffer)
    {
        read_buffer_size=protocal->getSliceSize();
        read_buffer.reset(new uint8_t[read_buffer_size],std::default_delete<uint8_t[]>());
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
        auto result=readFromBuf(read_buffer.get(),ret);
        if(result<0)return 0;
    }
    return ret;
}


ssize_t IcacheBufferStream::readFromBuf(const void *data,uint32_t buf_len)
{
    if(!appendCache(data,buf_len))
    {
        AIMY_ERROR("apeend cache failed!");
        return -1;
    }
    while(1)
    {
        auto ret=parseCache();
        if(ret.second==0)break;
        if(ret.second>protocal->getMaxFrameSize())
        {// treat it as an error
            AIMY_ERROR("frame size overflosw [%u>%u]",ret.second,protocal->getMaxFrameSize());
            return -1;
        }
        updateFrameCache(ret.first,ret.second);
    }
    return buf_len;
}

ssize_t IcacheBufferStream::sendCacheByFd()
{//try to send a frame
    if(frame_cache_map.size()==0)return -1;
    int64_t ret=0;
    auto frame=frame_cache_map.begin();
    while(frame->second>0)
    {
        // send by slice size
        //auto send_size=frame.second>protocal->getSliceSize()?protocal->getSliceSize():frame.second;
        //send as more as possible
        auto send_size=frame->second;
        auto send_ret=::send(fd,cache_buf.get()+frame->first,send_size,0);
        if(send_ret<0)
        {
            int err=platform::getErrno();
            if(err!=EAGAIN&&err!=EINTR)
            {
                return 0;
            }
            break;
        }
        else if (send_ret==0) {
            return 0;
        }
        frame->first+=send_ret;
        frame->second-=send_ret;
        ret+=send_ret;
    }
    if(frame->second==0)frame_cache_map.pop_front();
    if(ret==0)ret=-1;
    return ret;
}
