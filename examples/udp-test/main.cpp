#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/udp-echo-server.h"
#include "imp/commandlineTool.h"
using namespace aimy;
int udp_test(int argc,char *argv[]);
int netlink_test(int argc,char *argv[]);
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
    return  netlink_test(argc,argv);
    //return object_test(argc,argv);
    return udp_test(argc,argv);
}
int udp_test(int argc,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto t=loop.getTaskScheduler();
    if(argc==1)
    {
        UdpEchoServer server(t.get(),"0.0.0.0",9000);
        server.notifySpeedKb.connectFunc([](float send_kb,float recv_kb){
            AIMY_INFO("send:%fkB/s recv:%fkB/s",send_kb,recv_kb);
        });
        server.start();
        loop.waitStop();
    }
    else {
        std::string ip=argv[1];
        auto t2=loop.getTaskScheduler();
        UdpEchoServer client(t2.get(),"0.0.0.0",0);
        client.notifySpeedKb.connectFunc([](float send_kb,float recv_kb){
            AIMY_INFO("send:%fkB/s recv:%fkB/s",send_kb,recv_kb);
        });
        auto timer=t2->addTimer(100);
        if(argc<3||argc>3)timer->setSingle(true);
        if(argc>3)
        {
            client.start();
        }
        uint64_t cnt=0;
        timer->timeout.connectFunc([&](){
            std::string data=std::to_string(cnt++);

            std::string data2(32*1024-16,'c');
            for (size_t i =0;i<data.size();++i)
            {
                data2[i]=data[i];
            }
            client.sendData(ip.c_str(),9000,data2.c_str(),data2.length());
        });
        timer->start();
        loop.waitStop();
    }
    return 0;
}
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>

// little helper to parsing message using netlink macroses
void parseRtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));

    while (RTA_OK(rta, len)) {  // while not end of the message
        if (rta->rta_type <= max) {
            tb[rta->rta_type] = rta; // read attr
        }
        rta = RTA_NEXT(rta,len);    // get next attr
    }
}
//man 7 netlink
//man rtnetlink
int netlink_test(int argc ,char *argv[])
{
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);   // create netlink socket

    if (fd < 0) {
        printf("Failed to create netlink socket: %s\n", (char*)strerror(errno));
        return 1;
    }

    struct sockaddr_nl  local;  // local addr struct
    char buf[8192];             // message buffer
    struct iovec iov;           // message structure
    iov.iov_base = buf;         // set message buffer as io
    iov.iov_len = sizeof(buf);  // set size

    memset(&local, 0, sizeof(local));

    local.nl_family = AF_NETLINK;       // set protocol family
    local.nl_groups =   RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;   // set groups we interested in
    local.nl_pid = getpid();    // set out id using current process id

    // initialize protocol message header
    struct msghdr msg;
    {
        msg.msg_name = &local;                  // local address
        msg.msg_namelen = sizeof(local);        // address size
        msg.msg_iov = &iov;                     // io vector
        msg.msg_iovlen = 1;                     // io size
    }

    if (bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0) {     // bind socket
        printf("Failed to bind netlink socket: %s\n", (char*)strerror(errno));
        close(fd);
        return 1;
    }

    // read and parse all messages from the
    while (1) {
        ssize_t status = recvmsg(fd, &msg, MSG_DONTWAIT);

        //  check status
        if (status < 0) {
            if (errno == EINTR || errno == EAGAIN)
            {
                usleep(250000);
                continue;
            }

            printf("Failed to read netlink: %s", (char*)strerror(errno));
            continue;
        }

        if (msg.msg_namelen != sizeof(local)) { // check message length, just in case
            printf("Invalid length of the sender address struct\n");
            continue;
        }

        // message parser
        struct nlmsghdr *h;

        for (h = (struct nlmsghdr*)buf; status >= (ssize_t)sizeof(*h); ) {   // read all messagess headers
            int len = h->nlmsg_len;
            int l = len - sizeof(*h);
            char *ifName;

            if ((l < 0) || (len > status)) {
                printf("Invalid message length: %i\n", len);
                continue;
            }

            // now we can check message type
            if ((h->nlmsg_type == RTM_NEWROUTE) || (h->nlmsg_type == RTM_DELROUTE)) { // some changes in routing table
                printf("Routing table was changed\n");
            } else {    // in other case we need to go deeper
                char *ifUpp;
                char *ifRunn;
                struct ifinfomsg *ifi;  // structure for network interface info
                struct rtattr *tb[IFLA_MAX + 1];

                ifi = (struct ifinfomsg*) NLMSG_DATA(h);    // get information about changed network interface

                parseRtattr(tb, IFLA_MAX, IFLA_RTA(ifi), h->nlmsg_len);  // get attributes

                if (tb[IFLA_IFNAME]) {  // validation
                    ifName = (char*)RTA_DATA(tb[IFLA_IFNAME]); // get network interface name
                }

                if (ifi->ifi_flags & IFF_UP) { // get UP flag of the network interface
                    ifUpp = (char*)"UP";
                } else {
                    ifUpp = (char*)"DOWN";
                }

                if (ifi->ifi_flags & IFF_RUNNING) { // get RUNNING flag of the network interface
                    ifRunn = (char*)"RUNNING";
                } else {
                    ifRunn = (char*)"NOT RUNNING";
                }

                char ifAddress[256];    // network addr
                struct ifaddrmsg *ifa; // structure for network interface data
                struct rtattr *tba[IFA_MAX+1];

                ifa = (struct ifaddrmsg*)NLMSG_DATA(h); // get data from the network interface

                parseRtattr(tba, IFA_MAX, IFA_RTA(ifa), h->nlmsg_len);

                if (tba[IFA_LOCAL]) {
                    inet_ntop(AF_INET, RTA_DATA(tba[IFA_LOCAL]), ifAddress, sizeof(ifAddress)); // get IP addr
                }

                switch (h->nlmsg_type) { // what is actually happenned?
                    case RTM_DELADDR:
                        printf("Interface %s: address was removed\n", ifName);
                        break;

                    case RTM_DELLINK:
                        printf("Network interface %s was removed\n", ifName);
                        break;

                    case RTM_NEWLINK:
                        printf("New network interface %s, state: %s %s\n", ifName, ifUpp, ifRunn);
                        break;

                    case RTM_NEWADDR:
                        printf("Interface %s: new address was assigned: %s\n", ifName, ifAddress);
                        break;
                }
            }

            status -= NLMSG_ALIGN(len); // align offsets by the message length, this is important

            h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));    // get next message
        }

        usleep(250000); // sleep for a while
    }

    close(fd);  // close socket

    return 0;
}
