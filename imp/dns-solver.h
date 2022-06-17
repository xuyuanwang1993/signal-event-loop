#ifndef DNSSOLVER_H
#define DNSSOLVER_H
namespace aimy {
int DNS_Resolve(const char *domain,char *return_ip,const char * dns_server_ip, unsigned short dns_port,unsigned int micro_timeout);
}
#endif // DNSSOLVER_H
