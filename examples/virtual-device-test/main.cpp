#include "log/aimy-log.h"
#include "core/core-include.h"
#include "VirtualDevice/Machine.h"
#include "VirtualDevice/Effector.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualMachine/protocol.h"
#include "src/ServieUdpTest.h"
#include "imp/commandlineTool.h"
#include "third_party/json/cjson-interface.h"
#include "imp/network/itcp-server.h"
#include "imp/network/tcp-connection-manager.h"
#include "src/protocal_vd_stream.h"
#include<stdio.h>
using namespace aimy;
int vd_test(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/updater","updater");
    auto print_log=getenv("AIMY_DEBUG");
    if(print_log==nullptr||strcasecmp(print_log,"false")!=0)
    {
        fprintf(stderr,"you can export AIMY_DEBUG=false to disable default log message\r\n");
        aimy::AimyLogger::Instance().set_log_to_std(true);
    }
    else {
        aimy::AimyLogger::Instance().set_log_to_std(false);
    }
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    return vd_test(argc,argv);
}
int vd_test(int argc,char *argv[])
{
    VirtualDevice::Machine machine;
    VirtualDevice::Machine::Get()->Open(nullptr);
    commandLineTestTool tool;
    uint16_t in_port=9926;
    uint16_t out_port=9927;
    auto str_tmp=getenv("UDP_INPORT");
    if(str_tmp)
    {
        uint64_t temp=std::stoul(str_tmp);
        if(temp>0&&temp<65536)in_port=temp&0xffff;
    }
    str_tmp=getenv("UDP_OUTPORT");
    if(str_tmp)
    {
        uint64_t temp=std::stoul(str_tmp);
        if(temp>0&&temp<65536)out_port=temp&0xffff;
    }
    AIMY_DEBUG("you can use export UDP_INPORT and UDP_OUTPORT to set udp work port!");
    EventLoop loop(1);
    loop.start();
    UdpTestClient client(loop.getTaskScheduler().get(),"127.0.0.1",out_port);
    TcpConnectionManager manager(loop.getTaskScheduler().get());
    auto protocal=std::make_shared<virtual_device::ProtocalStream>();
    ITcpServer tcp_client(loop.getTaskScheduler().get(),"0");
    std::weak_ptr<TcpConnection>conn;

    tcp_client.notifyActiveConnection.connectFunc([&](SOCKET fd,uint32_t token,int64_t cost_ms){
        std::string host=NETWORK_UTIL::get_peer_ip(fd);
        uint16_t port=NETWORK_UTIL::get_peer_port(fd);
        AIMY_DEBUG("connect to %s %hu %d token:%u %ld ms",host.c_str(),port,fd,token,cost_ms);
        std::shared_ptr<TcpConnection>tcp_conn(new TcpConnection(loop.getTaskScheduler().get(),fd));
        conn=tcp_conn;
        manager.addConnection(tcp_conn);
        tcp_conn->disconnected.connect(tcp_conn.get(),[&manager,fd](){
            manager.removeConnection(fd);
        });
        auto conn_ptr=tcp_conn.get();
        tcp_conn->notifyFrame.connectFunc([conn_ptr,protocal](std::shared_ptr<uint8_t> frame,uint32_t frame_len){
            auto peer_host=NETWORK_UTIL::get_peer_ip(conn_ptr->getFd());
            auto peer_port=NETWORK_UTIL::get_peer_port(conn_ptr->getFd());
            virtual_device::StreamFrame frame_decode;
            if(protocal->bufferToFrame(frame.get(),frame_len,&frame_decode))
            {
                AIMY_DEBUG("recv from %s:%hu size[%u] payload->\r\n%s",peer_host.c_str(),peer_port,frame_decode.header.getHeaderLen()
                           ,std::string(reinterpret_cast<const char *>(frame_decode.payload_header),frame_decode.header.getHeaderLen()).c_str());
            }
            else {
                AIMY_ERROR("recv from %s:%hu size:%u false frame format!",peer_host.c_str(),peer_port,frame_len);
            }
        });
        tcp_conn->setProtocal(protocal);
    });
    client.notifyMessage.connectFunc([&](std::string ip,uint16_t port,std::shared_ptr<uint8_t> buf,uint32_t len){
        const VDProtocol::FrameV1 *p_frame = (decltype(p_frame))buf.get();
        size_t body_length;

        switch (p_frame->stream_offset) {
            case 0:
                body_length = 0;
                break;
            case ~(decltype(p_frame->stream_offset))0:
                body_length = len - sizeof(*p_frame);
                break;
            default:
                body_length = p_frame->stream_offset;
        }
        neb::CJsonObject object(std::string((const char *)p_frame->body,body_length));
        AIMY_DEBUG("recv from %s:%hu len %lu \r\n%s",ip.c_str(),port,body_length,object.ToFormattedString().c_str());

    });
    client.start();
    tool.initServer("0.0.0.0",8889);
    tool.insertCallback("set_tone","float[db] string[path for tone]",[&](const ExternalParamList&paramlist)->std::string{
        auto iter=paramlist.begin();
        std::string item=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        float tone=std::stof(item);
        ++iter;
        item=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        std::string path=item;
        auto ret=VirtualDevice::Machine::Get()->SetTone(tone,path);
        return "SetTone "+std::to_string(tone)+" "+path+" "+std::to_string(ret);
    },2);
    tool.insertCallback("query_version","query all device version",[&](const ExternalParamList&paramlist)->std::string{
        neb::CJsonObject object;
        object.Add("sub_device","common");
        object.Add("command","query_version");
        AIMY_DEBUG("send  \r\n%s",object.ToFormattedString().c_str());
        auto str_json=object.ToString();
        size_t json_length=str_json.size();
        const size_t c_length_frame = sizeof(VDProtocol::FrameV1) + json_length ;
        VDProtocol::FrameV1* const p_frame = (decltype(p_frame))alloca(c_length_frame);
        p_frame->frame.version = 0;
        p_frame->compress_flag = 0;
        if (json_length)
            memcpy(p_frame->body, str_json.c_str(), json_length);
        p_frame->stream_offset = ~0;
        client.sendData("127.0.0.1",in_port,p_frame,c_length_frame);
        return "query_device send";
    },0);
    tool.insertCallback("A2BReinitDevice","reinit A2B DEVICE",[&](const ExternalParamList&paramlist)->std::string{
        neb::CJsonObject object;
        object.Add("sub_device","A2B");
        object.Add("command","ReinitDevice");
        AIMY_DEBUG("send  \r\n%s",object.ToFormattedString().c_str());
        auto str_json=object.ToString();
        size_t json_length=str_json.size();
        const size_t c_length_frame = sizeof(VDProtocol::FrameV1) + json_length ;
        VDProtocol::FrameV1* const p_frame = (decltype(p_frame))alloca(c_length_frame);
        p_frame->frame.version = 0;
        p_frame->compress_flag = 0;
        if (json_length)
            memcpy(p_frame->body, str_json.c_str(), json_length);
        p_frame->stream_offset = ~0;
        client.sendData("127.0.0.1",in_port,p_frame,c_length_frame);
        return "query_device send";
    },0);
    tool.insertCallback("generate_set_tone_config","generate set tone config file[麦克风 0-3 模拟1 模拟2 数字1 数字2|音乐 4 5 7 伴奏 原唱 网络 | 低 0 中2  高3]",[&](const ExternalParamList&paramlist)->std::string{
        //麦克风 0-3 模拟1 模拟2 数字1 数字2
        //音乐 4 5 7 伴奏 原唱 网络
        //低 0 中2  高3
        neb::CJsonObject ret;
        {
            neb::CJsonObject config_mic;
            neb::CJsonObject filter;
            std::set<int>pipe_set{0,1,2,3};
            for(auto i:pipe_set)
            {
                neb::CJsonObject pipe;
                pipe.Add("pipe",i);
                neb::CJsonObject band_list;
                band_list.Add(0);
                pipe.Add("band_list",band_list);
                filter.Add(pipe);
            }
            config_mic.Add("filter",filter);
            config_mic.SetSavePath("./config_mic.json");
            config_mic.SaveToFile();
            ret.Add("config_mic",config_mic);
        }
        {
            neb::CJsonObject config_music;
            neb::CJsonObject filter;
            std::set<int>pipe_set{4,5,7};
            for(auto i:pipe_set)
            {
                neb::CJsonObject pipe;
                pipe.Add("pipe",i);
                neb::CJsonObject band_list;
                band_list.Add(0);
                pipe.Add("band_list",band_list);
                filter.Add(pipe);
            }
            config_music.Add("filter",filter);
            config_music.SetSavePath("./config_music.json");
            config_music.SaveToFile();
            ret.Add("config_music",config_music);
        }
        AIMY_DEBUG("send  \r\n%s",ret.ToFormattedString().c_str());
        return ret.ToFormattedString().c_str();
    },0);
    auto generate_SetCompandorCurveWithGain_command_func=[](){
        neb::CJsonObject ret;
        ret.Add("module","OmapL138");
        ret.Add("address",0);
        ret.Add("command","SetCompandorCurveWithGain");
        ret.Add("source",0);
        {//points
            ret.Add("point_count",4);
            neb::CJsonObject points;
            {
                neb::CJsonObject point;
                point.Add("x",1.1);
                point.Add("y",-80.0);
                points.Add(point);
            }
            {
                neb::CJsonObject point;
                point.Add("x",2.1);
                point.Add("y",-70.0);
                points.Add(point);
            }
            {
                neb::CJsonObject point;
                point.Add("x",3.1);
                point.Add("y",-50.0);
                points.Add(point);
            }
            {
                neb::CJsonObject point;
                point.Add("x",8.1);
                point.Add("y",-20.0);
                points.Add(point);
            }
            ret.Add("points",points);
        }
        ret.Add("gain",-50.0);
        return ret;
    };
    tool.insertCallback("generate_SetCompandorCurveWithGain_command","generate command context",[&](const ExternalParamList&paramlist)->std::string{
        (void)paramlist;
        neb::CJsonObject ret=generate_SetCompandorCurveWithGain_command_func();
        AIMY_DEBUG("send  \r\n%s",ret.ToFormattedString().c_str());
        return ret.ToFormattedString().c_str();
    },0);
    tool.insertCallback("test_generate_SetCompandorCurveWithGain_command","send the command to server",[&](const ExternalParamList&paramlist)->std::string{
        auto tcp_conn=conn.lock();
        if(!tcp_conn)
        {
            return "connection is not built";
        }
        (void)paramlist;
        neb::CJsonObject ret=generate_SetCompandorCurveWithGain_command_func();
        auto head_str=ret.ToFormattedString();
        AIMY_DEBUG("send  \r\n%s",head_str.c_str());

        auto frame=protocal->packetFrame(head_str.c_str(),head_str.size(),nullptr,0);
        tcp_conn->sendFrame(frame.first.get(),frame.second);
        return ret.ToFormattedString().c_str();
    },0);
    tool.insertCallback("connect-server","string [ip] uint16_t [port]",[&](const ExternalParamList&paramlist)->std::string{
        auto tcp_conn=conn.lock();
        if(tcp_conn)
        {
            return "connection is established";
        }
        auto iter=paramlist.begin();
        std::string ip=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        ++iter;
        std::string port=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        tcp_client.activeConnect(ip,port,5000);
        return "success";
    },2);
    tool.insertCallback("disconnect-server","",[&](const ExternalParamList&paramlist)->std::string{
        auto tcp_conn=conn.lock();
        if(!tcp_conn)
        {
            return "connection is aborted";
        }
        manager.removeConnection(tcp_conn->getFd());
        return "success";
    },0);
    tool.start();
    tool.waitDone();
    loop.waitStop();
    return 0;
}
