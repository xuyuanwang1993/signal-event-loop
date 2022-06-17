#include "ServieUdpTest.h"
using namespace aimy;
UdpTestClient::UdpTestClient(TaskScheduler *parent, const std::string &ip, uint16_t port):Object(parent),notifyMessage(this),working(false)
{
    fd=NETWORK_UTIL::build_socket(NETWORK_UTIL::UDP);
    NETWORK_UTIL::bind(fd,ip,port);
    localIp=NETWORK_UTIL::get_local_ip(fd);
    localPort=NETWORK_UTIL::get_local_port(fd);
    channel=parent->addChannel(fd);

    channel.get()->bytesReady.connect(this,std::bind(&UdpTestClient::on_read,this));
    AIMY_INFO("server :%s %hu",localIp.c_str(),localPort);
}

ssize_t UdpTestClient::sendData(const std::string &ip,uint16_t port,const void *data,uint32_t len)
{
    if(!working){
        AIMY_WARNNING("client is not start");
        return -1;
    }
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    return sendto(fd,data,len,0,(sockaddr *)&addr,addrlen);
}

void UdpTestClient::start()
{
    channel->enableReading();
    channel->sync();
    working.exchange(true);
}

void UdpTestClient::stop()
{
    working.exchange(false);
    channel->disableReading();
    channel->sync();
}

void UdpTestClient::on_read()
{
    static const uint32_t read_buf_len=32*1024;
    char buf[read_buf_len]={0};
    struct sockaddr_in addr;
    socklen_t len=sizeof (sockaddr_in);
    while(1)
    {
        memset(buf,0,read_buf_len);
        auto ret=recvfrom(fd,buf,read_buf_len,0,(sockaddr *)&addr,&len);
        if(ret<=0)break;
        std::string ip=inet_ntoa(addr.sin_addr);
        uint16_t port=ntohs(addr.sin_port);
        std::shared_ptr<uint8_t>tmp(new uint8_t[ret+1],std::default_delete<uint8_t[]>());
        memset(tmp.get(),0,ret+1);
        memcpy(tmp.get(),buf,ret);
        notifyMessage(ip,port,tmp,ret);
    }
}

UdpTestClient::~UdpTestClient()
{
    if(fd>0)NETWORK_UTIL::close_socket(fd);
}
