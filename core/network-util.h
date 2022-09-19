#ifndef NETWORKUTIL_H
#define NETWORKUTIL_H
#include <cstdint>
#include <cstring>
#include<string>
#include<vector>
#include<string.h>
#include<list>
#if defined(__linux) || defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/un.h>
#define SOCKET int
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#elif defined(WIN32) || defined(_WIN32)
#define bzero(a,b) memset(a,0,b)
#define FD_SETSIZE      1024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#else

#endif
namespace aimy {
namespace util {
class NetworkUtil
{
public:
    typedef enum{
        TCP=SOCK_STREAM,
        UDP=SOCK_DGRAM,
    }SOCKET_TYPE;
public:
    /*创建socket*/
    static SOCKET build_socket(SOCKET_TYPE type);
    /*关闭socket*/
    static void close_socket(SOCKET sockfd);
    /*绑定ip端口*/
    static bool bind(SOCKET sockfd,const std::string&ip="0.0.0.0",uint16_t port=0);
    /*连接socket*/
    static bool connect(SOCKET sockfd,std::string ip,uint16_t port,uint32_t time_out_ms=0);
    /*设置阻塞模式*/
    static bool make_blocking(SOCKET sockfd);
    /*设置非阻塞模式*/
    static bool make_noblocking(SOCKET sockfd);
    static void set_reuse_addr(SOCKET fd);
    static void sock_set_v6only(SOCKET fd);
    /*获取本地绑定的端口*/
    static uint16_t get_local_port(SOCKET sockfd);
    /*获取本地绑定的IP*/
    static std::string get_local_ip(SOCKET sockfd);
    /*获取对端的端口*/
    static uint16_t get_peer_port(SOCKET sockfd);
    /*获取对端的IP*/
    static std::string get_peer_ip(SOCKET sockfd);
    static uint16_t get_service_port(sockaddr_in *addr);
    static std::string get_host_info(sockaddr_in *addr);
    static int get_socket_error( int fd);
    /*多播使能*/
    static bool ip_multicast_enable(SOCKET sockfd,std::string localIp);
    /*加入多播组*/
    static bool ip_add_membership(SOCKET sockfd,std::string multicastIp,std::string localIp);
    /*离开多播组*/
    static bool ip_drop_membership(SOCKET sockfd,std::string multicastIp,std::string localIp);
    /*获取本地IP列表*/
    struct net_interface_info{
        std::string dev_name;
        std::string netmask;
        std::string ip;
        std::string mac;
        net_interface_info(const std::string &_dev_name,const std::string &_netmask,const std::string & _ip,const std::string &_mac):dev_name(_dev_name),netmask(_netmask),ip(_ip)\
          ,mac(_mac){

        }
    };
    /*获取网络状态*/
    static std::list<net_interface_info> readNetworkInfo();
    /*设置TCP的keepalive*/
    static bool set_tcp_keepalive(SOCKET sockfd,bool flag,uint32_t try_seconds=30,uint32_t max_tries=4,uint32_t try_interval=5);
    /**
     * @brief set_tcp_nodelay
     * @param sockfd
     * @return
     * 设置TCP_NODELAY 选项，打开此选项时，小于mss的包也会立即发送，不打开时会使用nagle算法 在包大于mss或含有fin包时发送否则放入缓冲区
开启此选项会降低带宽利用率，提高数据实时性
     */
    static bool set_tcp_nodelay(SOCKET sockfd,bool enabled);
    /**
     * @brief set_tcp_quickack
     * @param sockfd
     * @param enabled
     * @return
     * 设置TCP_QUICKACK 选项，打开此选项时，会立即发送包确认的ack，关闭此选项，会延迟发送确认ack，可使多个ack一起合并发送，最大延时默认是40ms，可配置
开启此选项会降低带宽利用率，提高数据实时性
     */
    static bool set_tcp_quickack(SOCKET sockfd,bool enabled);
    /**
     * @brief set_tcp_cork
     * @param sockfd
     * @param enabled
     * @return
     * 设置TCP_CORK选项，打开此选项时，不会发送小于mss的包，若包小于mss将会超时发送 默认200ms
开启此选项会提高带宽利用率，降低数据实时性
     */
    static bool set_tcp_cork(SOCKET sockfd,bool enabled);
};
}
}
#undef NETWORK_UTIL
#define NETWORK_UTIL aimy::util::NetworkUtil
#endif // NETWORKUTIL_H
