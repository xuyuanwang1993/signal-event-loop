#include "icache_buffer_udp.h"

using namespace  aimy;
IcacheBufferUdp::IcacheBufferUdp(std::shared_ptr<ProtocalBase> _protocal, SOCKET _fd):IcacheBufferBase(_protocal,_fd,32*1024)
{

}

IcacheBufferUdp::~IcacheBufferUdp ()
{

}

ssize_t IcacheBufferUdp::readFromFd ()
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
    sockaddr_in addr;
    memset(&addr,0,sizeof (addr));
    socklen_t len=sizeof (addr);
    auto ret=::recvfrom(fd,read_buffer.get(),read_buffer_size,0,reinterpret_cast<sockaddr *>(&addr),&len);
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
        //append recv addr

        if(!appendCache(&addr,sizeof (addr)))return 0;
        if(readFromBuf(read_buffer.get(),ret)<0)return 0;
    }
    return ret;
}

ssize_t IcacheBufferUdp::sendCacheByFd()
{
    if(frame_cache_map.size()==0)return -1;
    sockaddr_in addr;
    socklen_t len=sizeof (addr);
    auto frame=frame_cache_map.front();
    memcpy(&addr,cache_buf.get()+frame.first,len);
    auto ret=::sendto(fd,cache_buf.get()+frame.first+len,frame.second-len,0,reinterpret_cast<const sockaddr *>(&addr),len);
    if(ret<0)
    {
        int err=platform::getErrno();
        if(err!=EAGAIN&&err!=EINTR)
        {
            return 0;
        }
    }
    if(ret>0)frame_cache_map.pop_front();
    return ret;
}
