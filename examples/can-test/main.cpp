#include "log/aimy-log.h"
#include "core/core-include.h"
#include "imp/can-utils/can-util.h"
#include "imp/can-utils/can-util-socket.h"
#include "imp/commandlineTool.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include<stdio.h>
using namespace aimy;
int can_test(int argc,char *argv[]);
int can_test_2(int argc,char *argv[]);
int can_recv_test();
int can_send_test();
int thread_test();
int object_test(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
    aimy::AimyLogger::Instance().set_log_file_size(200*1024*1024);
    auto print_log=getenv("AIMY_DEBUG");
    if(print_log==nullptr||strcasecmp(print_log,"false")!=0)
    {
        fprintf(stderr,"you can export AIMY_DEBUG=false to disable default log message\r\n");
        aimy::AimyLogger::Instance().set_log_to_std(true);
    }
    else {
        aimy::AimyLogger::Instance().set_log_to_std(false);
    }
#ifdef DEBUG
    aimy::AimyLogger::Instance().set_minimum_log_level(LOG_ERROR);
#endif
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    return  can_test(argc,argv);
    return  thread_test();
    return can_send_test();
    return can_test_2(argc,argv);
}
int can_test(int argc,char *argv[])
{
    EventLoop loop(2);

    loop.start();
    CanUtilSocket util(loop.getTaskScheduler().get(),5000);
    util.initCan();
    CanUtilSocket util2(loop.getTaskScheduler().get(),5000);
    util2.initCan();
    auto CAN_PROXY=getenv("CAN_PROXY");
    if(CAN_PROXY!=nullptr)
    {
        auto CAN_BAUDRATE=getenv("CAN_BAUDRATE");
        int can_baudrate=0;
        if(CAN_BAUDRATE)can_baudrate=std::stoi(CAN_BAUDRATE);
        if(can_baudrate<=0)can_baudrate=200000;

        std::string tty_1="";
        auto CAN_TTY_1=getenv("CAN_TTY_SEND");
        if(CAN_TTY_1)tty_1=CAN_TTY_1;
        if(tty_1.empty())tty_1="/dev/ttyUSB0";
        util.initCanProxy(tty_1.c_str(),1500000,can_baudrate);
        util.getCanproxy()->init_statistics_state(true,false);
        std::string tty_2="";
        auto CAN_TTY_2=getenv("CAN_TTY_RECV");
        if(CAN_TTY_2){
            tty_2=CAN_TTY_2;
        }
        else {
            tty_2="/dev/ttyUSB1";
        }
        if(!tty_2.empty())
        {
            util2.initCanProxy(tty_2.c_str(),1500000,can_baudrate);
            util2.getCanproxy()->init_statistics_state(false,true);
        }

    }
    else {
        util.startCan();
    }
    commandLineTestTool tool;
    tool.initServer("0.0.0.0");
    tool.insertCallback("testcmd","int[index]",[&](const ExternalParamList&paramlist){
        auto buf=paramlist.begin()->first;
        auto len=paramlist.begin()->second;
        std::string cmd(reinterpret_cast<const char *>(buf.get()),len);
        AIMY_DEBUG("recv cmd[%s]",cmd.c_str());
        util.sendTestControlCommand(static_cast<CanCommandType>(std::stoi(cmd)));
        return "success";
    },1);
    tool.insertCallback("testcmd2","string [physical desaddr] string[data want to send with hex FORMAT] example [2001 0400010103E003]",[&](const ExternalParamList&paramlist){
        auto iter=paramlist.begin();
        auto addr_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        if(addr_data.second<2)return "invalid addr";
        uint16_t addr=((addr_data.first.get()[0])<<8)|addr_data.first.get()[1];
        ++iter;
        auto send_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        util.sendFrameToSpecificPhysicalAddr(addr,send_data.first,send_data.second);
        return "success";
    },2);
    tool.insertCallback("testlight","string [lightaddr] string [operation_type hex] string [data hex] uint8_t [0/1(filldata)]",[&](const ExternalParamList&paramlist){
        auto iter=paramlist.begin();
        auto addr_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        if(addr_data.second<4)return "invalid light addr";
        uint32_t addr=((addr_data.first.get()[0])<<24)|(addr_data.first.get()[1]<<16)|(addr_data.first.get()[2]<<8)|(addr_data.first.get()[3]);
        ++iter;
        auto operation_type_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        ++iter;
        auto operation_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        ++iter;
        bool fill=iter->first.get()[0]!=0;
        util.sendLightControlCommand(static_cast<CanDeviceAddr>(addr),static_cast<CanLightOperationType>(operation_type_data.first.get()[0])
                ,operation_data.first.get()[0],fill);
        return "success";
    },4);
    tool.insertCallback("testpillar","string [pillaraddr] string [operation_type hex] string [data hex] uint8_t [0/1(filldata)]",[&](const ExternalParamList&paramlist){
        auto iter=paramlist.begin();
        auto addr_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        if(addr_data.second<4)return "invalid pillaraddrr";
        uint32_t addr=((addr_data.first.get()[0])<<24)|(addr_data.first.get()[1]<<16)|(addr_data.first.get()[2]<<8)|(addr_data.first.get()[3]);
        ++iter;
        auto operation_type_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        ++iter;
        auto operation_data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        ++iter;
        bool fill=iter->first.get()[0]!=0;
        util.sendPillarControlCommand(static_cast<CanDeviceAddr>(addr),static_cast<PillarCmdType>(operation_type_data.first.get()[0])
                ,operation_data.first.get()[0],fill);
        return "success";
    },4);
    tool.insertCallback("testbroadcast","string [hex data to send]",[&](const ExternalParamList&paramlist){
        auto iter=paramlist.begin();
        auto data=CanUtilSocket::convertHexstrToBytes(iter->first.get(),iter->second);
        uint16_t cmd=data.first.get()[0];
        if(data.second>1)cmd=(cmd<<8)|data.first.get()[1];
        util.sendBroadCastCommand(static_cast<BroadCastCommand>(cmd));
        return "success";
    },1);
    tool.insertCallback("testall","exec all test command",[&](const ExternalParamList&paramlist){
        util.sendTestControlCommand(C_CAN_ENTER_TEST_MODE);
        for (uint32_t i=C_CAN_REQUEST_CHECK_1P_GROUP;i<=C_CAN_SET_MAJOR_SPOTLIGHT_ALL;++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            util.sendTestControlCommand(static_cast<CanCommandType>(i));
        }
        util.sendTestControlCommand(C_CAN_EXIT_TEST_MODE);
        return "success";
    },0);
    tool.insertCallback("testbyfile","string [file path]read data from the file,every line is a single command,[hexstr_addr#hexstr_data]"
                                     "2001#0400010103E003",[&](const ExternalParamList&paramlist){
        auto iter=paramlist.begin();
        std::string file_name(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        AIMY_DEBUG("recv file_name[%s]",file_name.c_str());
        size_t max_len=4096;
        char buf[max_len];
        char *ptr=buf;
        FILE *fp=fopen(file_name.c_str(),"r");
        int read_len=0;
        memset(buf,0,4096);
        while((read_len=getline(&ptr,&max_len,fp))!=-1)
        {
            char first[64];
            char second[64];
            memset(first,0,64);
            memset(second,0,64);
            if(sscanf(buf,"%[^#]#%s",first,second)==2)
            {
                AIMY_DEBUG("addr=%s data=%s",first,second);
                auto addr_data=CanUtilSocket::convertHexstrToBytes(first,strlen(first));
                if(addr_data.second<2)continue;
                uint16_t addr=((addr_data.first.get()[0])<<8)|addr_data.first.get()[1];
                auto send_data=CanUtilSocket::convertHexstrToBytes(second,strlen(second));
                util.sendFrameToSpecificPhysicalAddr(addr,send_data.first,send_data.second);
            }
            memset(buf,0,4096);
        }
        fclose(fp);
        return "success";
    },1);
    tool.insertCallback("looptestbyfile","string [file path]read data from the file,every line is a single command,[hexstr_addr#hexstr_data]"
                                     "2001#0400010103E003 double[send_interval ms] int[send times]",[&](const ExternalParamList&paramlist){
#ifdef DEBUG
        can_recv_cnt=0;
        can_send_cnt=0;
#endif
        auto iter=paramlist.begin();
        std::string file_name(reinterpret_cast<const char *>(iter->first.get()),iter->second);
        AIMY_DEBUG("recv file_name[%s]",file_name.c_str());
        size_t max_len=4096;
        char buf[max_len];
        char *ptr=buf;
        FILE *fp=fopen(file_name.c_str(),"r");
        int read_len=0;
        memset(buf,0,4096);
        struct test_data{
            uint16_t addr;
            std::pair<std::shared_ptr<uint8_t>,uint32_t> send_data;
        };
        std::list<test_data> send_data_list;
        while((read_len=getline(&ptr,&max_len,fp))!=-1)
        {
            char first[64];
            char second[64];
            memset(first,0,64);
            memset(second,0,64);
            if(sscanf(buf,"%[^#]#%s",first,second)==2)
            {
                AIMY_DEBUG("addr=%s data=%s",first,second);
                auto addr_data=CanUtilSocket::convertHexstrToBytes(first,strlen(first));
                if(addr_data.second<2)continue;
                uint16_t addr=((addr_data.first.get()[0])<<8)|addr_data.first.get()[1];
                auto send_data=CanUtilSocket::convertHexstrToBytes(second,strlen(second));
                send_data_list.push_back(test_data{addr,send_data});
            }
            memset(buf,0,4096);
        }
        fclose(fp);
        if(!send_data_list.empty())
        {
            ++iter;
            auto send_inerval_ms=std::stold(commandLineTestTool::paramToString(*iter));
            ++iter;
            if(send_inerval_ms<=0||send_inerval_ms>100000)send_inerval_ms=1000;
            auto send_times=std::stoi(commandLineTestTool::paramToString(*iter));
            ++iter;
            if(send_times<=0)send_times=1;
            std::thread t([=,&util](){
                auto i=send_times;
                while(i>0)
                {
                    for(auto data:send_data_list)
                    {
                        util.sendFrameToSpecificPhysicalAddr(data.addr,data.send_data.first,data.send_data.second);
                        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(send_inerval_ms*1000)));
                    }
                    --i;
                }
            });
            t.detach();
        }
        return "success";
    },3);
    tool.start();
#ifdef DEBUG
    util.notifyFrameCountForTest.connectFunc([&](const uint32_t &count,const uint32_t &touchFrameCount,const uint32_t &boardFrameCount){
        AIMY_BACKTRACE("total:%u  board:%u touch:%u",count,boardFrameCount,touchFrameCount);
        ;
    });
#endif
    util.notifyInsertCoin.connectFunc([](){
        AIMY_WARNNING("notifyInsertCoin");
    });
    util.notifyBackgroundButtonPressed.connectFunc([](){
        AIMY_WARNNING("notifyBackgroundButtonPressed");
    });
    util.notify1PGroupSuccess.connectFunc([](){
        AIMY_WARNNING("notify1PGroupSuccess");
    });
    util.notify2PGroupSuccess.connectFunc([](){
        AIMY_WARNNING("notify2PGroupSuccess");
    });
    util.notifyFrontStageGroupSuccess.connectFunc([](){
        AIMY_WARNNING("notifyFrontStageGroupSuccess");
    });
    util.notifyBackStageGroupSuccess.connectFunc([](){
        AIMY_WARNNING("notifyBackStageGroupSuccess");
    });
    util.notifyCentralControlSuccess.connectFunc([](){
        AIMY_WARNNING("notifyCentralControlSuccess");
    });
    util.notifyTestSensorStatusChanged.connectFunc([](uint8_t BoardSensorId,bool status){
        AIMY_WARNNING("notifyTestSensorStatusChanged,sensor %02x ->%d",BoardSensorId,status);
    });
    int release=0;
    int pressed=0;
    util.notifyBoardFiledStatusChanged.connectFunc([&](BoardFiledType type,BoardFiledStatus status){
        if(status==BoardPressed)
        {
            pressed++;
        }
        else if (status==BoardReleased) {
            release++;
        }
        AIMY_WARNNING("notifyBoardFiledStatusChanged %02x->%02x  press:%d release:%d",type,status,pressed,release);
    });
    int release_2=1;
    int pressed_2=0;
    util.notifyScreenTouchEvent.connectFunc([&](uint16_t x_pos,uint16_t y_pos,ScreenTouchEvent event){
        if(event==ScreenPressed)
        {
            pressed_2++;
        }
        else if (event==ScreenReleased) {
            release_2++;
        }
        AIMY_WARNNING("notifyScreenTouchEvent %hu:%hu ->%02x press:%d release:%d",x_pos,y_pos,event,pressed_2,release_2);
    });
//    util.notifyCanDeviceErroForTest.connectFunc([&](){
//        AIMY_ERROR("restart can device!");

//        auto reset=getenv("AIMY_RESET_CAN");
//        if(reset==nullptr||strcasecmp(reset,"false")!=0)
//        {
//          fprintf(stderr,"you can export AIMY_RESET_CAN=true to enale reset can\r\n");
//        }
//        else {
//            fprintf(stderr,"reset can interface\r\n");
//            system("ip link set can0 down");
//            system("ip link set can0 up type can bitrate 200000 loopback off");
//        }
//    });
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
        tool.stop();
    });
    loop.waitStop();
    util.stopCan();
    return 0;
}

int can_test_2(int argc,char *argv[])
{
    CanUtilRaw util;
    util.initCan();
    util.startCan();
    auto cmd=static_cast<CanCommandType>(std::stoi(argv[1]));
    util.sendTestControlCommand(cmd);
    util.stopCan();
    return 0;
}

int can_recv_test()
{
    struct sockaddr_can addr;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval) + 3 * sizeof(struct timespec) + sizeof(__u32))];
    struct iovec iov;
    struct msghdr msg;
    struct canfd_frame frame;
    int nbytes;
    struct ifreq ifr;
    auto fd=socket(PF_CAN,SOCK_RAW,CAN_RAW);
    addr.can_family=AF_CAN;
    memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    strncpy(ifr.ifr_name, "can0", 4);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        exit(1);
    }
    addr.can_ifindex=0;
    uint32_t canfd_on=1;
    setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
            }
    iov.iov_base = &frame;
    msg.msg_name = &addr;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &ctrlmsg;
    while(1)
    {
        iov.iov_len = sizeof(frame);
        msg.msg_namelen = sizeof(addr);
        msg.msg_controllen = sizeof(ctrlmsg);
        msg.msg_flags = 0;
        nbytes = recvmsg(fd, &msg, 0);
        char buf[17]={0};
        uint32_t offset=0;
        for(int i=0;i<frame.len;++i)
        {
            sprintf(buf+offset,"%02X",frame.data[i]);
            offset+=2;
        }
        uint32_t can_id=frame.can_id;
        if (can_id & CAN_ERR_FLAG) {
            can_id &=(CAN_ERR_MASK|CAN_ERR_FLAG);
        } else if (can_id & CAN_EFF_FLAG) {
            can_id &=CAN_EFF_MASK;
        } else {
            can_id&=CAN_SFF_MASK;
        }
//        char buf_1[2]={0xFF,0XFB};
//        AIMY_INFO("recv %08X data[%u]->%s",can_id,frame.len,buf);
    }
    return 0;
}

int can_send_test()
{
    int s; /* can raw socket */
    struct sockaddr_can addr;
    struct can_frame frame;
    memset(&frame,0,sizeof (frame));
    struct ifreq ifr;
    frame.can_id=0X1FFFE021|CAN_EFF_FLAG;
    /* open socket */
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("socket");
        return 1;
    }
    printf("--------\r\n");
    frame.can_dlc=8;
    memset(frame.data,'F',8);
    strcpy((char *)(ifr.ifr_name), "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);

    memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;


    /* disable default receive filter on this RAW socket */
    /* This is obsolete as we do not read from the socket at all, but for */
    /* this reason we can remove the receive list in the Kernel to save a */
    /* little (really a very little!) CPU usage.                          */
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    int nbytes = send(s, &frame, sizeof(struct can_frame), 0);
    if(nbytes<0)
    {
        perror("send failed!");
    }
    close(s);

    return 0;
}

int thread_test()
{
    EventLoop loop(2,4);
    aimy::AimyLogger::Instance().register_exit_signal_func([&](){
        loop.stop();
    });
    loop.start();
    auto task1=[](const std::string &msg){
        sleep(10);
        AIMY_INFO("task1 %s",msg.c_str());
        return  msg;
    };
    auto ret=loop.getThreadPool()->buildTask(task1,"nihao");
    auto recv_func=[](const std::string &msg){
        AIMY_WARNNING("recv %s",msg.c_str());
    };
    auto ex_func=[](const std::string &msg){
        AIMY_WARNNING("exception %s",msg.c_str());
    };
    auto status_func=[](aimy::concurrent::TaskState state){
        AIMY_BACKTRACE("state:%d",state);
    };
    ret->notifyResult.connectFunc(recv_func);
    ret->notifyStatus.connectFunc(status_func);
    ret->notifyException.connectFunc(ex_func);
    loop.getThreadPool()->pushTask(ret);
    auto ret2=loop.getThreadPool()->buildTask(task1,"nihao22222");
    ret2->notifyStatus.connectFunc(status_func);
    loop.getThreadPool()->pushTask(ret2);
    ret2->cancelTask();

    auto ret3=loop.getThreadPool()->buildTask([](){
        throw std::runtime_error("test3");
    });
    ret3->notifyException.connectFunc(ex_func);
    ret3->notifyStatus.connectFunc(status_func);
    loop.getThreadPool()->pushTask(ret3);
    loop.waitStop();
    return 0;
}
int object_test(int argc,char *argv[])
{
    EventLoop loop(2);
    loop.start();
    Object object(loop.getTaskScheduler().get());
    Signal<>test(&object);
    test.connectFunc([](){
        printf("test call!\r\n");
    });
    object.addTriggerEvent([](){
        printf("-------try----------r\n");
    });
//    auto timer=loop.getTaskScheduler()->addTimer(1000);
//    timer->timeout.connectFunc([&](){
//        printf("emit--------\r\n");
//        test.emit();
//    });
//    timer->start();
    loop.waitStop();
    return 0;
}
