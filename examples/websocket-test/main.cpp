#include "log/aimy-log.h"
#include "third_party/json/cjson-interface.h"
#include "imp/network/itcp-server.h"
#include "imp/network/tcp-connection-manager.h"
#include<stdio.h>
#include "imp/network/websocket/websocket_connection_helper.h"
#include "imp/network/websocket/websocket_api_base.h"
#include <lua.hpp>
#include "sol.hpp"
using namespace aimy;
int lua_test(int argc ,char *argv[]);
int test(int argc ,char *argv[]);
sol::state lua;
std::atomic<bool> is_running=true;
std::mutex task_mutex;
std::queue<std::function<void()>> task_queue;
static void push_task(const std::function<void()> &task)
{
    std::lock_guard<std::mutex> locker(task_mutex);
    task_queue.push(task);
}
static void lua_notify_message(const std::string &message)
{
    push_task([message](){
        sol::function hand_message=lua["hand_message"];
        hand_message(message);
    });
}
static void log_message(const std::string &message)
{
    AIMY_DEBUG("%s",message.c_str());
}
class WebsocketApiTest:public WebsocketApiBase
{
public:
    WebsocketApiTest(const std::string &api_name, bool is_passive, TaskScheduler *_parent, SOCKET _fd):WebsocketApiBase(api_name,is_passive,_parent,_fd)
    {

    }
    virtual ~WebsocketApiTest(){}
    std::string description() const override
    {
        return "WS_T["+apiName+":"+getPeerHostName()+":"+std::to_string(getPeerPort())+"]";
    }
private:
    void handleTextFrame(const StreamWebsocketFrame &frame)override
    {
        std::string message=std::string( reinterpret_cast<const char *>( frame.payload),frame.header.actual_payload_len);
        AIMY_DEBUG("recv:%s",message.c_str());
        lua_notify_message(message);
    }
};
using namespace aimy;
using neb::CJsonObject;
static std::shared_ptr<EventLoop> p_loop=nullptr;
static std::shared_ptr<ITcpServer> p_tcp_server=nullptr;
static std::shared_ptr<TcpConnectionManager> p_vd_connection_manager=nullptr;
static std::shared_ptr<WebsocketApiTest> p_ws_conn=nullptr;
static int send_message(const std::string &message)
{
    AIMY_DEBUG("try_send[%p]:%s",p_ws_conn.get(),message.c_str());
    p_ws_conn->sendWebsocketFrame(WS_FrameType::WS_TEXT_FRAME,message.c_str(),message.length());
    return 0;
}
static void disconnec_connection()
{
    AIMY_DEBUG("%s",__func__);
    is_running.exchange(false);
    p_ws_conn->disconnected();
    _Exit(0);
}

static bool need_execed()
{
    return is_running;
}
static void sleep_ms(int ms)
{
    usleep(ms*1000);
}
static void handle_task()
{
    std::unique_lock<std::mutex> locker(task_mutex);
    while(!task_queue.empty())
    {
        auto task=task_queue.front();
        task_queue.pop();
        locker.unlock();
        task();
        locker.lock();
    }
}

void exec_websocket_test(const std::string&api_name,const std::string &ip,const std::string &port,const std::string &file_name)
{
    lua.open_libraries(sol::lib::base,sol::lib::string,sol::lib::package,sol::lib::math,sol::lib::table);
    lua.set_function("send_message",&send_message);
    lua.set_function("disconnec_connection",&disconnec_connection);
    lua.set_function("log_message",&log_message);
    lua.set_function("sleep_ms",&sleep_ms);
    lua.set_function("need_execed",&need_execed);
    lua.set_function("handle_task",&handle_task);
    lua.open_file(file_name.c_str());
    aimy::AimyLogger::Instance().register_handle();
#if defined(__ANDROID__)
#else
    aimy::AimyLogger::Instance().set_max_log_file_cnts(4);
    aimy::AimyLogger::Instance().set_log_file_size(20*1024*1024);
    aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/virtual_device/","vd_test");
#endif
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    p_loop.reset(new EventLoop(1,2));
    p_loop->start();//init_test_module
    if(!p_loop)return;
    // init command map,init authorized module
    p_vd_connection_manager.reset( new TcpConnectionManager(p_loop->getTaskScheduler().get()));
    p_tcp_server.reset(new ITcpServer(p_loop->getTaskScheduler().get(),"0"));

    AIMY_WARNNING("start command unit test");
    p_tcp_server->notifyActiveConnection.connectFunc([=](SOCKET fd,uint32_t token,int64_t cost_ms){
        std::string host=NETWORK_UTIL::get_peer_ip(fd);
        uint16_t port=NETWORK_UTIL::get_peer_port(fd);
        AIMY_DEBUG("connect to %s %hu %d token:%u %ld ms",host.c_str(),port,fd,token,cost_ms);
        std::shared_ptr<WebsocketConnectionHelper>http_conn(new WebsocketConnectionHelper(p_loop->getTaskScheduler().get(),fd));
        p_vd_connection_manager->addConnection(http_conn);
        http_conn->disconnected.connect(p_vd_connection_manager.get(),[fd](){
            p_vd_connection_manager->removeConnection(fd);
        });

        http_conn->setTimeout(5000);
        std::string api_name="/websocket";
        http_conn->activeSetup(api_name);
        http_conn->notifyNewWSConnection.connect(p_vd_connection_manager.get(),[=](std::string api,SOCKET ws_fd,bool is_passive){
            if(api!=api_name)
            {
                AIMY_ERROR("api:%s is not matched request %s",api.c_str(),api_name.c_str());
                NETWORK_UTIL::close_socket(ws_fd);
                return ;
            }
            AIMY_ERROR("passive:%s %d %d",api.c_str(),ws_fd,is_passive);


            p_ws_conn.reset(new WebsocketApiTest(api,is_passive,p_loop->getTaskScheduler().get(),ws_fd));
            p_vd_connection_manager->addConnection(p_ws_conn);
            p_ws_conn->disconnected.connect(p_vd_connection_manager.get(),[ws_fd](){
                p_vd_connection_manager->removeConnection(ws_fd);
                is_running.exchange(false);
                p_loop->stop();
            });
            std::thread ([=](){
                {//REQ_HEARTBEAT
                    neb::CJsonObject normal_group;
                    {
                        {
                            neb::CJsonObject group;
                            group.Add("group_name","root");
                            group.Add("key","rootaimy");
                            normal_group.Add(group);
                        }
                        {
                            neb::CJsonObject group;
                            group.Add("group_name","test");
                            group.Add("key","testaimy");
                            normal_group.Add(group);
                        }
                    }
                    CJsonObject request;
                    request.Add("cmd","req_heartbeat");
                    {
                        CJsonObject params;
                        params.Add("groups",normal_group);
                        request.Add("params",params);
                    }
                    AIMY_DEBUG("test_data--------\r\n%s",request.ToFormattedString().c_str());
                    std::string response_str=request.ToString();
                    while(1)
                    {
                        usleep(10*1000*1000);
                        p_ws_conn->sendWebsocketFrame(WS_FrameType::WS_TEXT_FRAME,response_str.c_str(),response_str.size());
                    }
                }
            }).detach();
            std::thread ([](){
                sol::function loop_task=lua["loop_task"];
                loop_task.call<void>();
            }).detach();
        });

    });
    p_tcp_server->activeConnect(ip,port,1000);
    p_loop->waitStop();
}

int main(int argc,char *argv[])
{
    std::string api_name="/websocket";
    if(argc>1)
    {
        api_name=argv[1];
    }
    AIMY_DEBUG("api_name:%s default[%s]",api_name.c_str(),"/websocket");
    std::string host="127.0.0.1";
    if(argc>2)
    {
        host=argv[2];
    }
    AIMY_DEBUG("host:%s default[%s]",host.c_str(),"127.0.0.1");
    std::string port="58090";
    if(argc>3)
    {
        port=argv[2];
    }
    AIMY_DEBUG("port:%s default[%s]",port.c_str(),"58090");
    std::string lua_file_path="./test.lua";
    if(argc>4)
    {
        lua_file_path=argv[3];
    }
    AIMY_DEBUG("lua_file_path:%s default[%s]",lua_file_path.c_str(),"./test.lua");
    exec_websocket_test(api_name,host,port,lua_file_path);
    return 0;
}
