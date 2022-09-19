#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/network/websocket/protocal-http.h"
#include "imp/network/tcp-connection-manager.h"
#include "imp/network/websocket/websocket_connection_helper.h"
#include "imp/network/itcp-server.h"
#include "imp/network/websocket/websocket_api_base.h"
using namespace aimy;
int test_1(int argc ,char *argv[]);
int test_2(int argc ,char *argv[]);
int main(int argc,char *argv[])
{
    return test_2(argc,argv);
}
int test_1(int argc ,char *argv[])
{
    {
        aimy::ProtocalHttp protocal;
        std::vector<std::string>header;
        header.push_back("GET");
        header.push_back("/chat");
        header.push_back("HTTP/1.1");

        std::map<std::string,std::string>keys;
        keys["Host"]="example.com:8000";
        keys["Connection"]="Upgrade";

        std::string payload="test";
        auto buffer=protocal.packetFrame(header,keys,nullptr,0);
        AIMY_INFO("\n%s",std::string(reinterpret_cast<const char *>(buffer.first.get()),buffer.second).c_str());
        {
            auto frame_status=protocal.decodeFrame(buffer.first.get(),buffer.second);
            AIMY_INFO("%d %d",frame_status.first,frame_status.second);
            aimy::StreamHttpFrame frame;
            protocal.bufferToFrame(buffer.first.get()+frame_status.first,frame_status.second,&frame);
            for(auto i :frame.header.header)
            {
                AIMY_INFO("%s",i.c_str());
            }
            for(auto i:frame.header.keys)
            {
                AIMY_INFO("%s:%s",i.first.c_str(),i.second.c_str());
            }
            if(frame.header.content_length>0)
            {
                AIMY_INFO("content:%s",std::string(reinterpret_cast<const char *>(frame.content),frame.header.content_length).c_str());
            }
        }
    }
    {
        aimy::ProtocalHttp protocal;
        std::vector<std::string>header;
        header.push_back("HTTP/1.1");
        header.push_back("101");
        header.push_back("Switch Protocals");

        std::map<std::string,std::string>keys;
        keys["Upgrade"]="websocket";
        keys["Connection"]="Upgrade";

        std::string payload="test";
        auto buffer=protocal.packetFrame(header,keys,nullptr,0);
        AIMY_INFO("\n%s",std::string(reinterpret_cast<const char *>(buffer.first.get()),buffer.second).c_str());
        {
            auto frame_status=protocal.decodeFrame(buffer.first.get(),buffer.second);
            AIMY_INFO("%d %d",frame_status.first,frame_status.second);
            aimy::StreamHttpFrame frame;
            protocal.bufferToFrame(buffer.first.get()+frame_status.first,frame_status.second,&frame);
            for(auto i :frame.header.header)
            {
                AIMY_INFO("%s",i.c_str());
            }
            for(auto i:frame.header.keys)
            {
                AIMY_INFO("%s:%s",i.first.c_str(),i.second.c_str());
            }
            if(frame.header.content_length>0)
            {
                AIMY_INFO("content:%s",std::string(reinterpret_cast<const char *>(frame.content),frame.header.content_length).c_str());
            }
        }
    }
    return 0;
}

int test_2(int argc,char *argv[])
{
    EventLoop loop(2);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto t=loop.getTaskScheduler();
    TcpConnectionManager manager(t.get());
    if(argc==1)
    {

        ITcpServer server(t.get(),"9000");
        server.notifyPassiveConnetion.connectFunc([&](SOCKET fd){
            AIMY_DEBUG("accept %d",fd);
            std::shared_ptr<WebsocketConnectionHelper> conn (new WebsocketConnectionHelper(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(&manager,[&manager,fd](){
                manager.removeConnection(fd);
            });
            conn->notifyNewWSConnection.connect(&manager,[&](std::string api,SOCKET i_fd,bool is_passive){
                AIMY_ERROR("passive:%s %d %d",api.c_str(),i_fd,is_passive);
                std::shared_ptr<WebsocketApiBase>conn(new WebsocketApiBase(api,is_passive,t.get(),i_fd));
                manager.addConnection(conn);
                conn->disconnected.connect(&manager,[&manager,i_fd](){
                    manager.removeConnection(i_fd);
                });
            });
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
            std::shared_ptr<WebsocketConnectionHelper>conn(new WebsocketConnectionHelper(t.get(),fd));
            manager.addConnection(conn);
            conn->disconnected.connect(&manager,[&manager,fd](){
                manager.removeConnection(fd);
            });
            conn->setTimeout(5000);
            conn->activeSetup("test");
            conn->notifyNewWSConnection.connect(&manager,[&](std::string api,SOCKET i_fd,bool is_passive){
                AIMY_ERROR("passive:%s %d %d",api.c_str(),i_fd,is_passive);
                std::shared_ptr<WebsocketApiBase>w_conn(new WebsocketApiBase(api,is_passive,t.get(),i_fd));
                manager.addConnection(w_conn);
                w_conn->disconnected.connect(&manager,[&manager,i_fd](){
                    manager.removeConnection(i_fd);
                });
                std::thread([=](){
                    std::string test("---------------------");
                    w_conn->sendWebsocketFrame(WS_FrameType::WS_TEXT_FRAME,test.c_str(),test.size());
                }).detach();
            });
        });
        AIMY_DEBUG("create connection task %u",client.activeConnect(host,port,5000));
        loop.waitStop();
    }
    return 0;
}