#include "uevent_monitor.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/un.h>
#include <linux/types.h>
#include <linux/netlink.h>
using namespace aimy;

UeventMonitor::UeventMonitor(TaskScheduler *parent):Object(parent),notifyUevent(this),scheduler(parent),channel(nullptr)
{

}

bool UeventMonitor::start()
{
    return invoke(Object::getThreadId(),[=]()->bool{
        bool ret=false;
        SOCKET sock=INVALID_SOCKET;
        do{
            if(channel){
                ret=true;
                break;
            }
            struct sockaddr_nl snl;
            int retval;
            int rcvbufsz = 128*1024;
            int rcvsz = 0;
            socklen_t prcvszsz = sizeof (rcvsz);
            const int feature_on = 1;

            // 初始化netlink socket的address
            memset(&snl, 0x00, sizeof(struct sockaddr_nl));
            snl.nl_family = AF_NETLINK;
            snl.nl_pid = getpid();
            snl.nl_groups = 0x01;

            /* 创建netlink socket：
            * 域：PF_NETLINK, 确定通信的特性，包括地址格式(struct sockaddr_nl)
            * 套接字类型：SOCK_DGRAM(无连接不可靠报文), 进一步确定通信特征
            * 协议：NETLINK_KOBJECT_UEVENT，对应内核里Kobject_uevent.c->netlink_kernel_create(net, NETLINK_KOBJECT_UEVENT, &cfg);
            */
            sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
            if (sock == -1) {
                AIMY_ERROR("error getting socket, [%s]",strerror(platform::getErrno()));
                break;
            }
            /*
            * try to avoid dropping uevents, even so, this is not a guarantee,
            * but it does help to change the netlink uevent socket's
            * receive buffer threshold from the default value of 106,496 to
            * the maximum value of 262,142.
            */
            retval = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbufsz, sizeof(rcvbufsz));
            if (retval < 0) {
                AIMY_ERROR("error setting receive buffer size for socket %s",platform::getErrno());
                break;
            }

            retval = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvsz, &prcvszsz);
            if (retval < 0) {
                AIMY_ERROR("error setting receive buffer size for socket,%s",platform::getErrno());
                break;
            }
            /*  enable receiving of the sender credentials */
            setsockopt(sock, SOL_SOCKET, SO_PASSCRED, &feature_on, sizeof(feature_on));

            // 将地址和socket绑定
            retval = bind(sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
            if (retval < 0) {
                AIMY_ERROR("bind failed %s",platform::getErrno());
                break;
            }
            channel=scheduler->addChannel(sock);
            channel->bytesReady.connect(this,std::bind(&UeventMonitor::on_recv,this));
            channel->enableReading();
            channel->sync();
            ret=true;
        }while(0);
        if(!ret)NETWORK_UTIL::close_socket(sock);
        AIMY_DEBUG("start uevent monitor %p  %d",this,ret);
        return ret;
    });
}

void UeventMonitor::stop()
{
    invoke(Object::getThreadId(),[=](){
        if(channel)
        {
            channel->stop();
            NETWORK_UTIL::close_socket(channel->getFd());
        }
        channel.reset();
        AIMY_DEBUG("stop uevent monitor %p",this);
    });
}

UeventMonitor::~UeventMonitor()
{
    stop();
}

void UeventMonitor::on_recv()
{
    int i;
    char *pos;
    size_t bufpos;
    ssize_t buflen;
    char *buffer;
    struct msghdr smsg;     /* 多重缓冲区，参考apue-16.5章节*/
    struct iovec iov;
    struct cmsghdr *cmsg;   // control message header
    struct ucred *cred;
    char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
    static char buf[IHOTPLUG_BUFFER_SIZE + IOBJECT_SIZE];

    // 初始化struct iovec
    memset(buf, 0x00, sizeof(buf));
    iov.iov_base = &buf;
    iov.iov_len = sizeof(buf);

    // 初始化struct msghdr
    memset (&smsg, 0x00, sizeof(struct msghdr));
    smsg.msg_iov = &iov;            /* array of I/O buffers */
    smsg.msg_iovlen = 1;            /* number of elements in array */
    smsg.msg_control = cred_msg;    /* ancillary data, 辅助数据 */
    smsg.msg_controllen = sizeof(cred_msg);

    buflen = recvmsg(channel->getFd(), &smsg, 0);
    if (buflen < 0) {
        return;
    }

    cmsg = CMSG_FIRSTHDR(&smsg);
    if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
        return;
    }

    cred = (struct ucred *)CMSG_DATA(cmsg);
    if (cred->uid != 0) {
        return;
    }

    /* skip header */
    bufpos = strlen(buf) + 1;
    if (bufpos < sizeof("a@/d") || bufpos >= sizeof(buf)) {
        return;
    }

    /* check message header */
    if (strstr(buf, "@/") == NULL) {
        return;
    }

    std::shared_ptr<Iuevent> uev(new Iuevent);

    if (!uev) {
        return;
    }

    if ((size_t)buflen > sizeof(buf)-1)
        buflen = sizeof(buf)-1;

    /*
     * Copy the shared receive buffer contents to buffer private
     * to this uevent so we can immediately reuse the shared buffer.
     */
    memcpy(uev->buffer, buf, IHOTPLUG_BUFFER_SIZE + IOBJECT_SIZE);
    buffer = uev->buffer;
    buffer[buflen] = '\0';

    AIMY_DEBUG("recv uevent : %s", buffer);

    /* save start of payload */
    bufpos = strlen(buffer) + 1;

    /* action string */
    uev->action = buffer;
    pos = strchr(buffer, '@');
    if (!pos) {
        return;
    }
    pos[0] = '\0';

    /* sysfs path */
    uev->devpath = &pos[1];

    /* hotplug events have the environment attached - reconstruct envp[] */
    /* uevent数据的后面会跟着环境变量 */
    for (i = 0; (bufpos < (size_t)buflen) && (i < IHOTPLUG_NUM_ENVP-1); i++) {
        int keylen;
        char *key;

        key = &buffer[bufpos];
        keylen = strlen(key);
        uev->envp[i] = key;
        bufpos += keylen + 1;
    }
    uev->envp[i] = NULL;
    notifyUevent(uev);
}
