#include "device-discover.h"
#include <ifaddrs.h>
#define RESPONSE_STR "Response"
using namespace aimy;
DeviceDiscover::DeviceDiscover(TaskScheduler *parent, const std::string & agent, bool enableRecv, const std::string &ip, uint16_t port):Object(parent),notifyMessage(this),userAgent(agent)
  ,multicastip(ip),multicastPort(port)
{
    fd=NETWORK_UTIL::build_socket(NETWORK_UTIL::UDP);
    if(enableRecv){
        NETWORK_UTIL::bind(fd,"0.0.0.0",port);
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof on);
        bool flag=true;
        setsockopt(fd,IPPROTO_IP,IP_MULTICAST_LOOP, (char*)&flag,sizeof (flag));
        uint8_t ttl=2;
        setsockopt(fd,IPPROTO_IP,IP_MULTICAST_TTL, (char*)&ttl,sizeof(ttl));

        NETWORK_UTIL::ip_multicast_enable(fd,"0.0.0.0");

        struct ip_mreq mreq;
        bzero(&mreq,sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr(multicastip.c_str());
        mreq.imr_interface.s_addr = inet_addr("0.0.0.0");
        setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP, (char*)&mreq,sizeof(mreq));
#ifdef IP_MULTICAST_ALL
        int multicastAll = 0;
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_ALL, (void*)&multicastAll, sizeof multicastAll);
#endif
    }
    //init ip update timer
    updateTimer=this->addTimer(2000);
    updateTimer->setSingle(false);
    updateTimer->timeout.connect(this,std::bind(&DeviceDiscover::on_timeout,this));

    channel=parent->addChannel(fd);
    channel.get()->bytesReady.connect(this,std::bind(&DeviceDiscover::on_read,this));
}

void DeviceDiscover::sendDiscover(std::string match_string, std::string specificIp)
{
    CJsonObject object;
    object.Add("cmd","discover");
    object.Add("agent",userAgent);
    object.Add("match_str",match_string);
    std::string data=object.ToString();
    if(specificIp.empty())
    {
        sendData(data.c_str(),data.length(),multicastip.c_str(),multicastPort);
    }
    else {
        sendData(data.c_str(),data.length(),specificIp.c_str(),multicastPort);
    }
}

void DeviceDiscover::sendData(const void *data,uint32_t len,const std::string&des_ip,uint16_t des_port)
{
    struct sockaddr_in sockaddr;
    bzero(&sockaddr,sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(des_port);
    //IP格式转换
    inet_pton(AF_INET,(char *)des_ip.c_str(),&sockaddr.sin_addr);
    ::sendto(fd,data,len,0,(struct sockaddr *)&sockaddr,sizeof(sockaddr));
}

void DeviceDiscover::sendData(const void *data,uint32_t len,const struct sockaddr_in &addr)
{
    ::sendto(fd,data,len,0,(struct sockaddr *)&addr,sizeof(addr));
}

void DeviceDiscover::start(std::shared_ptr<ThreadPool> pool)
{
    threadPool=pool;
    channel->enableReading();
    channel->sync();
}

void DeviceDiscover::stop()
{
    channel->disableReading();
    channel->sync();
}

void DeviceDiscover::setUpdateTimerState(bool enable)
{
    if(enable)
    {
        updateTimer->start();
    }
    else {
        updateTimer->stop();
    }
}

DeviceDiscover::~DeviceDiscover()
{

    if(fd>0){
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = inet_addr(multicastip.c_str());
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));
        NETWORK_UTIL::close_socket(fd);
    }
}

void DeviceDiscover::on_read()
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
        handleMessage(buf,read_buf_len,addr);
    }
}

void DeviceDiscover::handleMessage(const void *data,uint32_t len,const struct sockaddr_in &addr)
{
    std::string message(static_cast<const char *>(data),len);
    CJsonObject object(message);
    std::string cmd;
    std::string agent;
    std::string match_str;
    object.Get("cmd",cmd);
    object.Get("agent",agent);
    object.Get("match_str",match_str);
    if(agent!=userAgent)return;
    if(cmd=="discover")
    {
        if(threadPool)
        {
            threadPool->enqueue([this,addr,cmd,match_str](){
                auto network_info=NETWORK_UTIL::readNetworkInfo();
                CJsonObject response;
                response.Add("cmd",cmd+RESPONSE_STR);
                response.Add("agent",DEFAULT_AGENT);
                packetBufWithShellCommand("uname -a",response,"kernel");
                packetBufWithShellCommand("cat /etc/issue",response,"rootfs");
                packetBufWithShellCommand("uptime",response,"uptime");
                CJsonObject interfaces;
                for(auto i:network_info)
                {
                    CJsonObject interface;
                    interface.Add("dev_name",i.dev_name);
                    interface.Add("ip",i.ip);
                    interface.Add("mac",i.mac);
                    interface.Add("netmask",i.netmask);
                    interfaces.Add(interface);
                }
                response.Add("interfaces",interfaces);
                std::string respons_str=response.ToFormattedString();
                if(match_str.empty()||(!match_str.empty()&&respons_str.find(match_str)!=std::string::npos))
                {
                    sendData(respons_str.c_str(),respons_str.length(),addr);
                }
            });
        }
    }
    else {
        notifyMessage(message);
    }
}

void DeviceDiscover::packetBufWithShellCommand(const std::string &shellcommand,CJsonObject &object,const std::string & name)
{
    FILE *fp=popen(shellcommand.c_str(),"r");
    char buf[1024];
    memset(buf,0,1024);
    auto read_len=fread(buf,1,1024,fp);
    if(read_len>0)
    {
        object.Add(name,std::string(buf,read_len));
    }
    else {
        object.Add(name,"unknown");
    }
    pclose(fp);
}

void DeviceDiscover::on_timeout()
{
#ifndef __ANDROID__
   //get local ip set
    struct ifaddrs *p_addrs = NULL;
    if (getifaddrs(&p_addrs) != 0)
    {
        AIMY_ERROR("get ip config failed![%s]",strerror(errno));
    }
    std::set<std::string> new_ip_set;
    for (struct ifaddrs *cur = p_addrs; cur != NULL; cur = cur->ifa_next) {
        if (cur->ifa_addr && cur->ifa_addr->sa_family == AF_INET && strcmp(cur->ifa_name, "lo")) {
            struct sockaddr_in *addr = (struct sockaddr_in *)cur->ifa_addr;
            new_ip_set.insert(inet_ntoa(addr->sin_addr));
        }
    }
    if(p_addrs)freeifaddrs(p_addrs);
    //remove old ip
    for(auto i:localIpSet)
    {
        if(new_ip_set.find(i)==new_ip_set.end())
        {
            NETWORK_UTIL::ip_drop_membership(fd,multicastip,i);
        }
    }
    //add new ip
    for(auto i : new_ip_set)
    {
        if(localIpSet.find(i)==localIpSet.end())
        {
            NETWORK_UTIL::ip_add_membership(fd,multicastip,i);
        }
    }
    //swap
    std::swap(localIpSet,new_ip_set);
#endif
}
