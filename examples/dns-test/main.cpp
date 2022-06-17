#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/dns-solver.h"
using namespace aimy;
int dns_test(int argc,char *argv[]);
std::string parase_domain(const std::string &domain_info)
{
    struct sockaddr_in addr;
    if(inet_pton(AF_INET,domain_info.c_str(),&addr)!=0)
    {
        return domain_info;
    }
    std::string ret=std::string();
    struct addrinfo hints;
    struct addrinfo *result;
    int  s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    s = getaddrinfo(domain_info.c_str(), nullptr, &hints, &result);
    if (s != 0) {
        return ret;
    }
    ret=inet_ntoa((reinterpret_cast<struct sockaddr_in *>(result->ai_addr))->sin_addr);
    freeaddrinfo(result);
    return ret;
}
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    //return object_test(argc,argv);
    return dns_test(argc,argv);
}
int dns_test(int argc,char *argv[])
{
    char ip[24];
    DNS_Resolve(argv[1],ip,"10.0.3.254",53,5000000);
    AIMY_ERROR("ip:%s",parase_domain(argv[1]).c_str());
    return 0;
}
