#ifndef ITCPSERVER_H
#define ITCPSERVER_H
#include "core/core-include.h"
namespace aimy {
class ITcpServer final:public Object{
    struct ConnectionTask{
        uint32_t token;
        SOCKET fd;
        std::shared_ptr<IoChannel> channel;
        std::shared_ptr<Timer> timer;
        int64_t timestamp;
        ~ConnectionTask()
        {
            AIMY_DEBUG("release active connection task fd:%d token:%u",fd,token);
        }
    };
public:
    /**
     * @brief notifyPassiveConnetion send this signal while a passive connection is established
     */
    Signal<SOCKET> notifyPassiveConnetion;
    /**
     * @brief notifyActiveConnection send this signal while an active connection is finished
     * fd   >0 for success
     * token  to specify which connection,it's returned with activeConnect calling
     * cost_time <0  active release >=0 cost time
     */
    Signal<SOCKET /*fd*/,uint32_t/*token*/,int64_t /*cost_time  ms*/> notifyActiveConnection;
public:
    ITcpServer(TaskScheduler *_parent, const std::string &service);
    bool startListen(uint32_t max_pending_size=20);
    void stopListen();
    void stopAll();
    std::list<SOCKET> serverFd();
    std::string serviceName()const;
    /**
     * @brief activeConnect
     * @param host   examples: x.x.x.x | 127.0.0.1 |  2000::1:2345:6789:abcd ||  ::1
     * @param service  examples:8080 | http | ssh
     * @param timeout_msec specify the connection timeout time  millisecond
     * @return 0 for failed  >0 for success
     *  you can't pass hostname to host param
     */
    uint32_t activeConnect(const std::string &host,const std::string &service,uint32_t timeout_msec=30000);
    void cancelActiveConnecttion(uint32_t token);
    ~ITcpServer();
private:
    void on_accept(int fd);
    void release_task(std::shared_ptr<ConnectionTask> task);
private:
    TaskScheduler * const scheduler;
    const std::string service_name;
    std::list<std::shared_ptr<IoChannel>> server_channel_list;
//
    uint32_t next_token;

    std::unordered_map<SOCKET,std::shared_ptr<ConnectionTask>> connections_map;
    std::unordered_map<uint32_t,SOCKET> token_directory;
};
}
#endif // ITCPSERVER_H
