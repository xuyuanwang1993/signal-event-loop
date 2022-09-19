#include "network-util.h"
#include "platform.h"
#include "log/aimy-log.h"
#if defined(__linux) || defined(__linux__)
#include <netinet/tcp.h>
#endif
using namespace aimy::util;
SOCKET NetworkUtil::build_socket(NetworkUtil::SOCKET_TYPE type)
{
    auto ret=socket(AF_INET,type,0);
    if(ret==INVALID_SOCKET)
    {
        AIMY_ERROR("create socket[%d] failed[%s]",type,strerror(aimy::platform::getErrno()));
    }
    return ret;
}

void NetworkUtil::close_socket(SOCKET sockfd)
{
#if defined(__linux) || defined(__linux__)
    ::close(sockfd);
#elif defined(WIN32) || defined(_WIN32)
    ::closesocket(sockfd);
#endif
}

bool NetworkUtil::bind(SOCKET sockfd,const std::string&ip,uint16_t port)
{
    struct sockaddr_in addr;
    bzero(&addr,sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if(::bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == SOCKET_ERROR)
    {
        AIMY_ERROR("bind %d to %s %hu failed[%s]",sockfd,ip.c_str(),port,strerror(aimy::platform::getErrno()));
        return false;
    }
    return true;
}

bool NetworkUtil::connect(SOCKET sockfd,std::string ip,uint16_t port,uint32_t time_out_ms)
{
    bool isConnected = true;
    if (time_out_ms > 0)
    {
        make_noblocking(sockfd);
    }
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (::connect(sockfd, (struct sockaddr*)&addr, addrlen) == SOCKET_ERROR)
    {
        if (time_out_ms > 0)
        {
            isConnected = false;
            fd_set fdWrite;
            FD_ZERO(&fdWrite);
            FD_SET(sockfd, &fdWrite);
            struct timeval tv = { static_cast<__time_t>(time_out_ms / 1000), static_cast<__suseconds_t>(time_out_ms % 1000 * 1000 )};
            select(sockfd + 1, nullptr, &fdWrite, nullptr, &tv);
            if (FD_ISSET(sockfd, &fdWrite))
            {
                isConnected = true;
            }
            else {
                isConnected=false;
            }
            make_blocking(sockfd);
        }
        else
        {
            isConnected = false;
        }
    }
    if(!isConnected){
        AIMY_ERROR("connect %d to %s %hu %ums failed[%s]",sockfd,ip.c_str(),port,time_out_ms,strerror(aimy::platform::getErrno()));
    }
    return isConnected;
}

bool NetworkUtil::make_blocking(SOCKET sockfd)
{
    bool result=false;
#if defined(__WIN32__) || defined(_WIN32)
    unsigned long arg = 0;
    result = ioctlsocket(sockfd, FIONBIO, &arg) == 0;
#elif defined(__linux) || defined(__linux__)
    int curFlags = fcntl(sockfd, F_GETFL, 0);
    result = fcntl(sockfd, F_SETFL, curFlags&(~O_NONBLOCK)) >= 0;
#endif
    return result;
}

bool NetworkUtil::make_noblocking(SOCKET sockfd)
{
#if defined(__linux) || defined(__linux__)
    int flags = fcntl(sockfd, F_GETFL, 0);
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#else
    unsigned long on = 1;
    return ioctlsocket(sockfd, FIONBIO, &on);
#endif
}

void NetworkUtil::set_reuse_addr(SOCKET fd)
{
    int on = 1;

    /*
         * Set socket options.
         * Allow local port reuse in TIME_WAIT.
         */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        AIMY_ERROR("setsockopt SO_REUSEADDR fd %d: %s", fd, strerror(platform::getErrno()));
}

void
NetworkUtil::sock_set_v6only(SOCKET fd)
{
#if defined(IPV6_V6ONLY) && !defined(__OpenBSD__)
    int on = 1;

    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) == -1)
        AIMY_ERROR("setsockopt IPV6_V6ONLY: %s", strerror(errno));
#endif
}

uint16_t NetworkUtil::get_local_port(SOCKET sockfd)
{
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return get_service_port(&addr);

    }
    AIMY_ERROR("getsockname failed [%s]",strerror(platform::getErrno()));
    return 0;
}

/*获取本地绑定的IP*/
std::string NetworkUtil::get_local_ip(SOCKET sockfd)
{
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return get_host_info(&addr);
    }
    AIMY_ERROR("getsockname failed [%s]",strerror(platform::getErrno()));
    return "0.0.0.0";
}

/*获取对端的端口*/
uint16_t NetworkUtil::get_peer_port(SOCKET sockfd)
{
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return get_service_port(&addr);
    }
    AIMY_ERROR("getsockname failed [%s]",strerror(platform::getErrno()));
    return 0;
}

/*获取对端的IP*/
std::string NetworkUtil::get_peer_ip(SOCKET sockfd)
{
    struct sockaddr_in addr ;
    bzero(&addr,sizeof (addr));
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return get_host_info(&addr);
    }
    AIMY_ERROR("getsockname failed [%s]",strerror(platform::getErrno()));
    return "0.0.0.0";
}

uint16_t NetworkUtil::get_service_port(sockaddr_in *addr)
{
    switch (addr->sin_family) {
    case AF_INET6:
        return ntohs(((sockaddr_in6 *)addr)->sin6_port);
    default:
        return ntohs(addr->sin_port);
    }
}

std::string NetworkUtil::get_host_info(sockaddr_in *addr)
{
    char buf[INET6_ADDRSTRLEN];
    void *numericAddress;
    switch (addr->sin_family) {
    case AF_INET6:
        numericAddress=&((struct sockaddr_in6 *)addr)->sin6_addr;
        break;
    default:
        numericAddress=&(addr->sin_addr);
        break;
    }
    std::string ret="";
    if(inet_ntop(addr->sin_family,numericAddress,buf,sizeof (buf))==nullptr)
    {
        AIMY_ERROR("inet_ntop failed [%s]",strerror(platform::getErrno()));
    }
    else {
        ret=buf;
    }
    return ret;
}

int NetworkUtil::get_socket_error(int fd)
{
    int socket_error=0;
    socklen_t size=sizeof (socket_error);
    getsockopt(fd,SOL_SOCKET,SO_ERROR,&socket_error,&size);
    return socket_error;
}

bool NetworkUtil::ip_multicast_enable(SOCKET sockfd,std::string localIp)
{
    in_addr addr={0};
    addr.s_addr=inet_addr(localIp.c_str());
    return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof(addr))==0;
}

bool NetworkUtil::ip_add_membership(SOCKET sockfd,std::string multicastIp,std::string localIp)
{
    struct ip_mreq req;
    memset(&req, 0, sizeof(req));
    req.imr_multiaddr.s_addr = inet_addr(multicastIp.c_str());
    req.imr_interface.s_addr = inet_addr(localIp.c_str());;
    return setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req))==0;
}

bool NetworkUtil::ip_drop_membership(SOCKET sockfd,std::string multicastIp,std::string localIp)
{
    struct ip_mreq req;
    memset(&req, 0, sizeof(req));
    req.imr_multiaddr.s_addr = inet_addr(multicastIp.c_str());
    req.imr_interface.s_addr = inet_addr(localIp.c_str());;
    return setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&req, sizeof(req))==0;
}

std::list<NetworkUtil::net_interface_info> NetworkUtil::readNetworkInfo()
{
    std::list<NetworkUtil::net_interface_info> ret;
    do{
#if defined(__linux) || defined(__linux__)
        SOCKET sockfd = INVALID_SOCKET;
        char buf[512] = { 0 };
        struct ifconf ifconf;
        struct ifreq  *ifreq;
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == INVALID_SOCKET)
        {
            close(sockfd);
            break;
        }
        ifconf.ifc_len = 512;
        ifconf.ifc_buf = buf;
        if (ioctl(sockfd, SIOCGIFCONF, &ifconf) < 0)
        {
            close(sockfd);
            break;
        }
        ifreq = (struct ifreq*)ifconf.ifc_buf;
        for (int i = (ifconf.ifc_len / sizeof(struct ifreq)); i>0; i--)
        {
            if (ifreq->ifr_flags == AF_INET)
            {
                if (strcmp(ifreq->ifr_name, "lo") != 0)
                {
                    struct ifreq ifr2;
                    strcpy(ifr2.ifr_name,ifreq->ifr_name);
                    uint8_t mac[6]={0};
                    char mac1[128]={0};
                    if((ioctl(sockfd,SIOCGIFHWADDR,&ifr2) )== 0)
                    {//获取mac ,前6个字节
                        memcpy(mac,ifr2.ifr_hwaddr.sa_data,6);
                        sprintf(mac1,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
                    }
                    std::string netmask;
                    if((ioctl(sockfd,SIOCGIFNETMASK,&ifr2) )== 0)
                    {//获取netmask ,前6个字节
                        netmask=inet_ntoa(((struct sockaddr_in*)&(ifr2.ifr_netmask))->sin_addr);
                    }
                    ret.push_back(net_interface_info(ifreq->ifr_name,netmask,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr),mac1));

                }
                ifreq++;
            }
        }
        close(sockfd);
#elif defined(WIN32) || defined(_WIN32)
        //              PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
        //              unsigned long size = sizeof(IP_ADAPTER_INFO);

        //              int ret = GetAdaptersInfo(pIpAdapterInfo, &size);
        //              if (ret == ERROR_BUFFER_OVERFLOW)
        //              {
        //                  delete pIpAdapterInfo;
        //                  pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[size];
        //                  ret = GetAdaptersInfo(pIpAdapterInfo, &size);
        //              }

        //              if (ret != ERROR_SUCCESS)
        //              {
        //                  delete pIpAdapterInfo;
        //                  break;
        //              }
        //              auto p_iter = pIpAdapterInfo;
        //              while (p_iter)
        //              {
        //                  IP_ADDR_STRING *pIpAddrString = &(p_iter->IpAddressList);
        //                  while(pIpAddrString)
        //                  {
        //                      if (strcmp(pIpAddrString->IpAddress.String, "127.0.0.1")!=0
        //                              && strcmp(pIpAddrString->IpAddress.String, "0.0.0.0")!=0)
        //                      {
        //                          char buffer[20];
        //                          sprintf_s(buffer, "%.2x-%.2x-%.2x-%.2x-%.2x-%.2x", p_iter->Address[0],
        //                                  p_iter->Address[1], p_iter->Address[2], p_iter->Address[3],
        //                                  p_iter->Address[4], p_iter->Address[5]);
        //                          if(pIpAdapterInfo->Type== MIB_IF_TYPE_ETHERNET)m_net_interface_info_cache.push_back(net_interface_info("MIB_IF_TYPE_ETHERNET", pIpAddrString->IpAddress.String,buffer));
        //                          else if(pIpAdapterInfo->Type == IF_TYPE_IEEE80211)m_net_interface_info_cache.push_back(net_interface_info("IF_TYPE_IEEE80211", pIpAddrString->IpAddress.String,buffer));
        //                          else m_net_interface_info_cache.push_back(net_interface_info("UNKNOWN", pIpAddrString->IpAddress.String));
        //                          break;
        //                      }
        //                      pIpAddrString = pIpAddrString->Next;
        //                  };
        //                  p_iter = p_iter->Next;
        //              }

        //              delete []pIpAdapterInfo;
#endif
    }while (0);

    return ret;
}

bool NetworkUtil::set_tcp_keepalive(SOCKET sockfd,bool flag,uint32_t try_seconds,uint32_t max_tries,uint32_t try_interval)
{
    if(flag){
#if defined(__WIN32__) || defined(_WIN32)
        // How do we do this in Windows?  For now, just make this a no-op in Windows:
#else
        int const keepalive_enabled = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive_enabled, sizeof keepalive_enabled) < 0) {
            return false;
        }

#ifdef TCP_KEEPIDLE
        uint32_t  keepalive_time = try_seconds==0?180:try_seconds;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepalive_time, sizeof keepalive_time) < 0) {
            return false;
        }
#endif

#ifdef TCP_KEEPCNT
        uint32_t  keepalive_count = max_tries==0?5:max_tries;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepalive_count, sizeof keepalive_count) < 0) {
            return false;
        }
#endif

#ifdef TCP_KEEPINTVL
        uint32_t  keepalive_interval = try_interval==0?60:try_interval;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepalive_interval, sizeof keepalive_interval) < 0) {
            return false;
        }
#endif
#endif
        return true;
    }else {
#if defined(__WIN32__) || defined(_WIN32)
        return true;
#else
        int const keepalive_enabled = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive_enabled, sizeof keepalive_enabled) < 0) {
            return false;
        }
        return true;
#endif
    }
}

bool NetworkUtil::set_tcp_nodelay(SOCKET sockfd,bool enabled)
{
#if defined(__linux) || defined(__linux__)
#ifdef TCP_NODELAY
    int option=enabled?1:0;
    return  setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,&option,sizeof (option));
#endif
#endif
    return false;
}

bool NetworkUtil::set_tcp_quickack(SOCKET sockfd,bool enabled)
{
#if defined(__linux) || defined(__linux__)
#ifdef TCP_QUICKACK
    int option=enabled?1:0;
    return  setsockopt(sockfd,IPPROTO_TCP,TCP_QUICKACK,&option,sizeof (option));
#endif
#endif
    return false;
}

bool NetworkUtil::set_tcp_cork(SOCKET sockfd,bool enabled)
{
#if defined(__linux) || defined(__linux__)
#ifdef TCP_CORK
    int option=enabled?1:0;
    return  setsockopt(sockfd,IPPROTO_TCP,TCP_CORK,&option,sizeof (option));
#endif
#endif
    return false;
}