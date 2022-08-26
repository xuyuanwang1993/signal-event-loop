#include "itcp-server.h"
#include <random>
using namespace aimy;
ITcpServer::ITcpServer(TaskScheduler *_parent,const std::string &service):Object(_parent),notifyPassiveConnetion(this),notifyActiveConnection(this),scheduler(_parent),service_name(service)
  ,next_token(0)
{
    server_channel_list.clear();
}

bool ITcpServer::startListen(uint32_t max_pending_size)
{
    if(serviceName().empty())return false;
    return invoke(Object::getCurrentThreadId(),[this,max_pending_size](){
        if(!server_channel_list.empty())return true;
        struct addrinfo addr_guess;
        memset(&addr_guess,0,sizeof (addr_guess));
        addr_guess.ai_family=AF_UNSPEC;
        addr_guess.ai_flags=AI_PASSIVE;
        addr_guess.ai_socktype=SOCK_STREAM;
        addr_guess.ai_protocol=IPPROTO_TCP;
        struct addrinfo *server_addr;
        auto ret=getaddrinfo(nullptr,service_name.c_str(),&addr_guess,&server_addr);
        if(ret!=0)
        {
            AIMY_ERROR("getaddrinfo for %s failed! [%s]",service_name.c_str(),gai_strerror(ret));
            return false;
        }
        auto ptr=server_addr;
        for(;ptr;ptr=ptr->ai_next)
        {
            auto fd=::socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);
            if(fd<0)
            {
                AIMY_ERROR("[%s] create socket failed![%s]",service_name.c_str(),strerror(platform::getErrno()));
                continue;
            }
            NETWORK_UTIL::set_reuse_addr(fd);
            if(ptr->ai_family==AF_INET6)
            {//otherwise binding call will be failed with the message "address is already in used"
                NETWORK_UTIL::sock_set_v6only(fd);
            }
            if((bind(fd,ptr->ai_addr,ptr->ai_addrlen)==0)&&
                    (listen(fd,max_pending_size)==0))
            {
                auto channel=scheduler->addChannel(fd);
                channel->bytesReady.connectFunc([this,fd](){
                    on_accept(fd);
                });
                channel->enableReading();
                channel->sync();
                server_channel_list.push_back(channel);
                AIMY_DEBUG("init tcp server [%s:%hu] success",NETWORK_UTIL::get_local_ip(fd).c_str(),NETWORK_UTIL::get_local_port(fd));
            }
            else {
                NETWORK_UTIL::close_socket(fd);
                AIMY_ERROR("[%s] bind or listen failed![%s]",service_name.c_str(),strerror(platform::getErrno()));
            }
        }
        while(ptr)
        AIMY_BACKTRACE("");
        freeaddrinfo(ptr);
        return !server_channel_list.empty();
    });
}

void ITcpServer::stopListen()
{
    invoke(Object::getCurrentThreadId(),[this](){
        for(auto i :server_channel_list)
        {
            i->stop();
            NETWORK_UTIL::close_socket(i->getFd());
        }
        server_channel_list.clear();
    });
}

void ITcpServer::stopAll()
{
    return invoke(Object::getCurrentThreadId(),[this](){
        stopListen();
        for(auto &i :connections_map)
        {
            i.second->channel->stop();
            i.second->timer->release();
            token_directory.erase(i.second->token);
            NETWORK_UTIL::close_socket(i.second->fd);
            notifyActiveConnection(-1,i.second->token,-1);
        }
        connections_map.clear();
    });
}

std::list<SOCKET> ITcpServer::serverFd()
{
    return invoke(Object::getCurrentThreadId(),[this](){
        std::list<SOCKET> ret;
        for(auto i:server_channel_list)
        {
            ret.push_back(i->getFd());
        }
        return ret;
    });
}

std::string ITcpServer::serviceName()const
{
    return service_name;
}

uint32_t ITcpServer::activeConnect(const std::string &host,const std::string &service,uint32_t timeout_msec)
{
    AIMY_DEBUG("try connect to host:%s port:%s %ldms",host.c_str(),service.c_str(),timeout_msec);
    return invoke(Object::getCurrentThreadId(),[=](){
        uint32_t token=0;
        do{
            struct addrinfo addr_guess;
            memset(&addr_guess,0,sizeof (addr_guess));
            addr_guess.ai_family=AF_UNSPEC;
            addr_guess.ai_flags=AI_NUMERICHOST;
            addr_guess.ai_socktype=SOCK_STREAM;
            addr_guess.ai_protocol=IPPROTO_TCP;

            struct addrinfo *client_addr;
            auto ret=getaddrinfo(host.c_str(),service.c_str(),&addr_guess,&client_addr);
            if(ret!=0)
            {
                AIMY_ERROR("getaddrinfo for %s failed! [%s]",service.c_str(),gai_strerror(ret));
                break;
            }
            //find token
            uint32_t max_try_times=1000;
            token=next_token;
            while(max_try_times-->0)
            {
                if(token==0)++token;
                if(token_directory.find(token)==token_directory.end())break;
                if(max_try_times>900)++token;
                else if(max_try_times>500){
                    token+=100;
                }
                else if (max_try_times>100) {
                    token+=200;
                }
                else {
                    std::random_device rd;
                    token+=(rd()%100)+1;
                }
            }
            if(max_try_times==0)token=0;
            if(token>0)
            {
                //init socket
                SOCKET fd=-1;
                auto ptr=client_addr;
                while(ptr)
                {
                    fd=::socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);
                    if(fd<0)
                    {
                        AIMY_ERROR("[%s] create socket failed![%s]",service.c_str(),strerror(platform::getErrno()));
                        ptr=ptr->ai_next;
                        continue;
                    }
                    break;
                }
                if(fd<0)token=0;
                else {
                    std::shared_ptr<ConnectionTask>task(new ConnectionTask());
                    std::weak_ptr<ConnectionTask> task_w(task);
                    task->fd=fd;
                    task->token=token;
                    task->channel=scheduler->addChannel(fd);
                    task->timer=addTimer(timeout_msec);
                    task->channel->enablWriting();
                    task->timestamp=Timer::getTimeNow<std::chrono::milliseconds>();
                    task->timer->timeout.connectFunc([task,this](){
                        AIMY_ERROR("connect failed [%s]",strerror(NETWORK_UTIL::get_socket_error(task->fd)));
                        notifyActiveConnection(-1,task->token,task->timer->getInterval());
                        release_task(task);
                        NETWORK_UTIL::close_socket(task->fd);
                    });
                    task->channel->writeReady.connectFunc([task,this](){
                        release_task(task);
                        auto cost_time=Timer::getTimeNow()-task->timestamp;
                        if(cost_time<0)cost_time=-1;
                        NETWORK_UTIL::set_tcp_keepalive(task->fd,true);
                        notifyActiveConnection(task->fd,task->token,cost_time);
                    });
                    task->channel->errorEvent.connectFunc([task,this](){
                        AIMY_ERROR("connect failed [%s]",strerror(NETWORK_UTIL::get_socket_error(task->fd)));
                        release_task(task);
                        NETWORK_UTIL::close_socket(task->fd);
                        auto cost_time=Timer::getTimeNow()-task->timestamp;
                        if(cost_time<0)cost_time=-1;
                        notifyActiveConnection(-1,task->token,cost_time);
                    });
                    task->channel->closeEvent.connectFunc([task,this](){
                        AIMY_ERROR("connect failed [%s]",strerror(NETWORK_UTIL::get_socket_error(task->fd)));
                        release_task(task);
                        NETWORK_UTIL::close_socket(task->fd);
                        auto cost_time=Timer::getTimeNow()-task->timestamp;
                        if(cost_time<0)cost_time=-1;
                        notifyActiveConnection(-1,task->token,cost_time);
                    });
                    task->channel->sync();
                    task->timer->start();
                    //try connect
                    ::connect(fd,ptr->ai_addr,ptr->ai_addrlen);
                    AIMY_DEBUG("use %s:%hu  to connect %s:%hu",NETWORK_UTIL::get_local_ip(fd).c_str(),NETWORK_UTIL::get_local_port(fd)
                               ,NETWORK_UTIL::get_host_info((sockaddr_in *)(ptr->ai_addr)).c_str()
                               ,NETWORK_UTIL::get_service_port((sockaddr_in *)(ptr->ai_addr)));
                    next_token=token+1;
                    connections_map.emplace(fd,task);
                    token_directory.emplace(token,fd);
                }
            }
            freeaddrinfo(client_addr);
        }while(0);
        return token;
    });
}

void ITcpServer::cancelActiveConnecttion(uint32_t token)
{
    invoke(Object::getCurrentThreadId(),[this,token](){
        auto iter=token_directory.find(token);
        if(iter==token_directory.end())return ;
        auto iter2=connections_map.find(iter->second);
        if(iter2!=connections_map.end())
        {
            iter2->second->timer->release();
            iter2->second->channel->stop();
            NETWORK_UTIL::close_socket(iter2->second->fd);
            notifyActiveConnection(-1,iter2->second->token,-1);
            connections_map.erase(iter2);
        }
        token_directory.erase(iter);
    });
}

ITcpServer::~ITcpServer()
{
    stopAll();
}

void ITcpServer::on_accept(int fd)
{
    struct sockaddr_storage addr;
    socklen_t len=sizeof (addr);
    SOCKET ret=::accept(fd,reinterpret_cast<sockaddr *>(&addr),&len);
    if(ret<0)
    {
        AIMY_ERROR("[%s] accept failed![%s]",service_name.c_str(),strerror(platform::getErrno()));
    }
    NETWORK_UTIL::set_tcp_keepalive(ret,true);
    notifyPassiveConnetion(ret);
}

void ITcpServer::release_task(std::shared_ptr<ConnectionTask> task)
{
    task->timer->release();
    task->timer.reset();//avoid circular references
    task->channel->stop();
    task->channel->rleaseFd();//recyle fd
    task->channel.reset();//avoid circular references
    connections_map.erase(task->fd);
    token_directory.erase(task->token);
}
