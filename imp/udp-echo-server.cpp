#include "udp-echo-server.h"
#include "imp/network/protocal-normal.h"
#include "imp/network/protocal_stream.h"
using namespace aimy;
UdpEchoServer::UdpEchoServer(TaskScheduler *parent, const std::string &ip, uint16_t port):Object(parent),notifySpeedKb(this),protocal(new ProtocalNormal(20*32*1024,32*1024,32*1024-16))
  ,recvBytes(0),sendBytes(0)
{
    fd=NETWORK_UTIL::build_socket(NETWORK_UTIL::UDP);
    NETWORK_UTIL::bind(fd,ip,port);
    localIp=NETWORK_UTIL::get_local_ip(fd);
    localPort=NETWORK_UTIL::get_local_port(fd);
    channel=parent->addChannel(fd);
    channel.get()->bytesReady.connect(this,std::bind(&UdpEchoServer::on_read,this));
    channel.get()->writeReady.connect(this,std::bind(&UdpEchoServer::on_write,this));
    readCache.reset(new IcacheBufferUdp(protocal,fd));
    writeCache.reset(new IcacheBufferUdp(protocal,fd));
    channel->enableReading();
    channel->sync();

    //
    speedNotifyTimer=parent->addTimer(2000);
    speedNotifyTimer->timeout.connect(this,std::bind(&UdpEchoServer::on_timeout,this));
    speedNotifyTimer->start();
    AIMY_INFO("server %s:%hu",localIp.c_str(),localPort);
}

ssize_t UdpEchoServer::sendData(const std::string &ip,uint16_t port,const void *data,uint32_t len)
{
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    uint32_t frame_len=len+addrlen;
    std::shared_ptr<uint8_t>frame(new uint8_t[frame_len],std::default_delete<uint8_t>());
    memcpy(frame.get(),&addr,addrlen);
    memcpy(frame.get()+addrlen,data,len);
    return invoke(Object::getCurrentThreadId(),[=](){
        auto ret=writeCache->appendFrame(frame.get(),frame_len);
        if(writeCache->frame_count()==1)
        {
            channel->enablWriting();
            channel->sync();
        }
        return ret;
    });
}

void UdpEchoServer::start()
{
    echoing=true;
}

void UdpEchoServer::stop()
{
    echoing=false;
}

void UdpEchoServer::on_read()
{
    int max_per_read_cnt=10;
    while(max_per_read_cnt-->0)
    {
        auto ret=readCache->readFromFd();
        if(ret<=0)break;
        //
        {
            recvBytes+=ret;
        }
        auto frame=readCache->popFrame();
        auto data=frame.first;
        auto data_len=frame.second;
        struct sockaddr_in addr ;
        memcpy(&addr,data.get(),sizeof (addr));
        std::string ip=inet_ntoa(addr.sin_addr);
        uint16_t port=ntohs(addr.sin_port);
        data_len-=sizeof (addr);
#ifdef DEBUG
        AIMY_DEBUG("recv from %s %hu len[%ld][%s]",ip.c_str(),port,data_len,std::string(reinterpret_cast<char *>(data.get())+sizeof (addr),data_len>1000?1000:data_len).c_str());
#endif
        if(echoing)
        {
            writeCache->appendFrame(frame.first.get(),frame.second);
            if(writeCache->frame_count()==1)
            {
                channel->enablWriting();
                channel->sync();
            }
        }
    }
}

void UdpEchoServer::on_write()
{

    while(writeCache->frame_count()>0)
    {
        auto ret=writeCache->sendCacheByFd();
        if(ret<=0)break;
        {
            sendBytes+=ret;
        }
    }
    if(writeCache->frame_count()==0)
    {
        channel->disableWriting();
        channel->sync();
    }
}

void UdpEchoServer::on_timeout()
{
    notifySpeedKb(sendBytes/(1024*2.0),recvBytes/(1024*2.0));
    sendBytes=0;
    recvBytes=0;
}


UdpEchoServer::~UdpEchoServer()
{
    if(fd>0)NETWORK_UTIL::close_socket(fd);
}
