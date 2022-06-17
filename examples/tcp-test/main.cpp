#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/network/itcp-server.h"
#include "imp/network/tcp-connection-manager.h"
#include "imp/network/protocal_stream.h"
#include "json/cjson-interface.h"
#include "imp/debugger/timer-escaper.h"
using namespace aimy;
using neb::CJsonObject;
int tcp_test(int argc,char *argv[]);
int tcp_test_2(int argc,char *argv[]);
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
    return tcp_test_2(argc,argv);
    return tcp_test(argc,argv);
}
int tcp_test(int argc,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto t=loop.getTaskScheduler();
    TcpConnectionManager manager(t.get());
    auto protocal=std::make_shared<ProtocalBase>(4096*10,true,4096,1400);
    if(argc==1)
    {

        ITcpServer server(t.get(),"9000");
        server.notifyPassiveConnetion.connectFunc([&](SOCKET fd){
            AIMY_DEBUG("accept %d",fd);
            std::shared_ptr<TcpConnection>conn(new TcpConnection(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(conn.get(),[&manager,fd](){
                manager.removeConnection(fd);
            });
            //此处禁止传递共享指针，应该传递原始指针或者弱引用
            auto conn_ptr=conn.get();
            conn->notifyFrame.connectFunc([conn_ptr](std::shared_ptr<uint8_t> frame,uint32_t frame_len){
                //echo
                auto peer_host=NETWORK_UTIL::get_peer_ip(conn_ptr->getFd());
                auto peer_port=NETWORK_UTIL::get_peer_port(conn_ptr->getFd());
                AIMY_INFO("recv from %s:%hu size:%u data:[%s]",peer_host.c_str(),peer_port,frame_len,std::string(reinterpret_cast<const char *>(frame.get()),frame_len>1500?1500:frame_len).c_str());
                conn_ptr->sendFrame(frame.get(),frame_len);
            });
            conn->setProtocal(protocal);
        });
        AIMY_DEBUG("listen ret %d ",server.startListen());
        loop.waitStop();
    }
    else {
        std::string host=argv[1];
        std::string port=argv[2];
        auto t2=loop.getTaskScheduler();
        ITcpServer client(t2.get(),"0");
        client.notifyActiveConnection.connectFunc([&](SOCKET fd,uint32_t token,int64_t cost_ms){
            AIMY_DEBUG("connect to %s %s %d token:%u %ld ms",host.c_str(),port.c_str(),fd,token,cost_ms);
            std::shared_ptr<TcpConnection>conn(new TcpConnection(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(conn.get(),[&manager,fd](){
                manager.removeConnection(fd);
            });
            auto conn_ptr=conn.get();
            conn->notifyFrame.connectFunc([conn_ptr](std::shared_ptr<uint8_t> frame,uint32_t frame_len){
                //echo
                auto peer_host=NETWORK_UTIL::get_peer_ip(conn_ptr->getFd());
                auto peer_port=NETWORK_UTIL::get_peer_port(conn_ptr->getFd());
                AIMY_INFO("recv from %s:%hu size:%u data:[%s]",peer_host.c_str(),peer_port,frame_len,std::string(reinterpret_cast<const char *>(frame.get()),frame_len>1500?1500:frame_len).c_str());
                conn_ptr->sendFrame(frame.get(),frame_len);
            });
            conn->setProtocal(protocal);
            std::string buf(1200,'a');
            conn->sendFrame(buf.c_str(),buf.size());
        });
        AIMY_DEBUG("create connection task %u",client.activeConnect(host,port,5000));
        loop.waitStop();
    }
    return 0;
}

int tcp_test_2(int argc,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto t=loop.getTaskScheduler();
    TcpConnectionManager manager(t.get());
    auto protocal=std::make_shared<ProtocalStream>();
    if(argc==1)
    {

        ITcpServer server(t.get(),"9000");
        server.notifyPassiveConnetion.connectFunc([&](SOCKET fd){
            AIMY_DEBUG("accept %d",fd);
            std::shared_ptr<TcpConnection>conn(new TcpConnection(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(conn.get(),[&manager,fd](){
                manager.removeConnection(fd);
            });
            //此处禁止传递共享指针，应该传递原始指针或者弱引用
            auto conn_ptr=conn.get();
            conn->notifyFrame.connectFunc([conn_ptr,protocal](std::shared_ptr<uint8_t> frame,uint32_t frame_len){
                //echo
                conn_ptr->sendFrame(frame.get(),frame_len);
                auto peer_host=NETWORK_UTIL::get_peer_ip(conn_ptr->getFd());
                auto peer_port=NETWORK_UTIL::get_peer_port(conn_ptr->getFd());
                StreamFrame frame_decode;
                if(protocal->bufferToFrame(frame.get(),frame_len,&frame_decode))
                {
                    std::string payload_header;
                    std::string payload;
                    if(frame_decode.header.header_length>0)
                    {
                        payload_header=std::string(reinterpret_cast<const char *>(frame_decode.payload_header),frame_decode.header.header_length);
                    }
                    if(frame_decode.header.payload_length>0)
                    {
                        payload=std::string(reinterpret_cast<const char *>(frame_decode.payload),frame_decode.header.payload_length);
                    }
                    AIMY_INFO("recv from %s:%hu size:%u header[%u]:[%s] payload[%u]:[%s]"
                              ,peer_host.c_str(),peer_port,frame_len,frame_decode.header.header_length,payload_header.c_str(),frame_decode.header.payload_length,payload.c_str());
                }
                else {
                    AIMY_ERROR("recv from %s:%hu size:%u false frame format!",peer_host.c_str(),peer_port,frame_len);
                }
            });
            conn->setProtocal(protocal);
        });
        AIMY_DEBUG("listen ret %d ",server.startListen());
        loop.waitStop();
    }
    else {
        std::string host=argv[1];
        std::string port=argv[2];
        auto t2=loop.getTaskScheduler();
        ITcpServer client(t2.get(),"0");
        client.notifyActiveConnection.connectFunc([&](SOCKET fd,uint32_t token,int64_t cost_ms){
            AIMY_DEBUG("connect to %s %s %d token:%u %ld ms",host.c_str(),port.c_str(),fd,token,cost_ms);
            std::shared_ptr<TcpConnection>conn(new TcpConnection(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(conn.get(),[&manager,fd](){
                manager.removeConnection(fd);
            });
            auto conn_ptr=conn.get();
            conn->notifyFrame.connectFunc([conn_ptr,protocal](std::shared_ptr<uint8_t> frame,uint32_t frame_len){
                auto peer_host=NETWORK_UTIL::get_peer_ip(conn_ptr->getFd());
                auto peer_port=NETWORK_UTIL::get_peer_port(conn_ptr->getFd());
                StreamFrame frame_decode;
                if(protocal->bufferToFrame(frame.get(),frame_len,&frame_decode))
                {
                    std::string payload_header;
                    std::string payload;
                    if(frame_decode.header.header_length>0)
                    {
                        payload_header=std::string(reinterpret_cast<const char *>(frame_decode.payload_header),frame_decode.header.header_length);
                    }
                    if(frame_decode.header.payload_length>0)
                    {
                        payload=std::string(reinterpret_cast<const char *>(frame_decode.payload),frame_decode.header.payload_length);
                    }
                    AIMY_INFO("recv from %s:%hu size:%u header[%u]:[%s] payload[%u]:[%s]"
                              ,peer_host.c_str(),peer_port,frame_len,frame_decode.header.header_length,payload_header.c_str(),frame_decode.header.payload_length,payload.c_str());
                    CJsonObject header(payload_header);
                    std::string cmd;
                    if(!header.Get("cmd",cmd))cmd="echo";
                    uint64_t count;
                    if(!header.Get("count",count))
                    {
                        count=0;
                    }
                    else {
                        count++;
                    }
                    CJsonObject send_object;
                    send_object.Add("cmd",cmd);
                    send_object.Add("count",count);
                    std::string header_str=send_object.ToString();
                    uint32_t payload_len=(count%10000000)+1;
                    std::shared_ptr<uint8_t>payload_buf(new uint8_t[payload_len],std::default_delete<uint8_t[]>());
                    memset(payload_buf.get(),'*',payload_len);
                    auto frame_new=protocal->packetFrame(header_str.c_str(),header_str.size(),payload_buf.get(),payload_len);
                    conn_ptr->sendFrame(frame_new.first.get(),frame_new.second);
                }
                else {
                    AIMY_ERROR("recv from %s:%hu size:%u false frame format!",peer_host.c_str(),peer_port,frame_len);
                }
            });
            conn->setProtocal(protocal);
            CJsonObject header;
            int count=9000000;
            header.Add("cmd","echo");
            header.Add("count",count);
            std::string header_str=header.ToString();
            std::shared_ptr<uint8_t>payload_buf(new uint8_t[count],std::default_delete<uint8_t[]>());
            memset(payload_buf.get(),'*',count);
            auto frame=protocal->packetFrame(header_str.c_str(),header_str.size(),payload_buf.get(),count);
            conn->sendFrame(frame.first.get(),frame.second);
        });
        AIMY_DEBUG("create connection task %u",client.activeConnect(host,port,5000));
        loop.waitStop();
    }
    return 0;
}
