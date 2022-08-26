#include "log/aimy-log.h"
#include "core/core-include.h"
#include "third_party/json/cjson-interface.h"
#include "imp/utils/common_utils.h"
#include "imp/common/serial_utils.h"
using namespace aimy;
//int main(int argc,char *argv[])
//{
//    (void)argc;
//    (void)argv;
//    aimy::AimyLogger::Instance().register_handle();
//    //aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
//    aimy::AimyLogger::Instance().set_log_to_std(true);
//    atexit([](){
//        aimy::AimyLogger::Instance().unregister_handle();
//    });
//    if(!AIMY_UTILS::acquireSigleInstanceLcok())
//    {
//        AIMY_ERROR("is running exit!");
//        return -1;
//    }
//    auto fd=aserial_open("/dev/ttyUSB0");
//    aserial_set_opt(fd,115200,8,'N',1);
//    std::thread t([&](){
//        uint8_t buffer[256];
//        while(1)
//        {
//            memset(buffer,0,256);
//            auto ret=read(fd,buffer,256);
//            if(ret>0)
//            {
//                printf("recv:");
//                for(auto i=0;i<ret;++i)
//                {
//                    printf("%02X ",buffer[i]);
//                }
//                printf("\r\n");
//            }
//        }
//    });
//    t.detach();
//    //获取模块的通道
////    uint32_t sleep_time=10;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x03,0x81,0xf3,0x01,0x02,0x00,0xa4,0x00};
//   // 1.获得版本信息
////    uint32_t sleep_time=10;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x03,0x81,0xf3,0x93,0x02,0x00,0xa4,0x00};
//    //3.获取模块的信号强度
////    uint32_t sleep_time=2000;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x03,0x81,0xf3,0x02,0x02,0x00,0xa4,0x00};
//    //4.配对
////        uint32_t sleep_time=5000;
////        bool pair_left=false;
////        bool pair_right=true;

////        uint8_t data_send[]={0x55,0xaa,0x08,0x05,0x81,0xf3,0x03,0x00,0x01,0x02,0x00,0xa4,0x00};
////        data_send[7]=pair_left?0x01:0x00;
////        data_send[8]=pair_right?0x01:0x00;

//    //频道扫描
////    uint32_t sleep_time=15000;
////    bool  is_right=false;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x05,0x81,0xf3,0x10,0x01,0x96,0x02,0x00,0xa4,0x00};
////    data_send[6]=data_send[6] |(is_right?0x1:0x0);
////    //设置频道
//    uint32_t sleep_time=15000;
//    bool  is_right=false;
//    uint8_t channel=0x0A;
//    uint8_t data_send[]={0x55,0xaa,0x08,0x08,0x81,0xf3,0x20,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0xa4,0x00};
//    data_send[6]=data_send[6] |(is_right?0x1:0x0);
//    data_send[7]=channel;
////搜索空闲频道
////    uint32_t sleep_time=15000;
////    bool  is_right=false;
////    uint8_t start_channel=1;
////    uint8_t end_channel=150;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x07,0x81,0xf3,0x30,0x00,0x00,0x0C,0x26,0x02,0x00,0xa4,0x00};
////    data_send[6]=data_send[6] |(is_right?0x1:0x0);
////    data_send[7]=start_channel;
////    data_send[8]=end_channel;
////设置发送频率
////        uint32_t sleep_time=1000;
////        bool  is_right=false;
////        uint8_t start_channel=1;
////        uint8_t end_channel=150;
////        uint32_t freq=600000;
////        uint8_t data_send[]={0x55,0xaa,0x08,0x07,0x81,0xf3,0x40,0x00,0x00,0x0C,0x26,0x02,0x00,0xa4,0x00};
////        data_send[6]=data_send[6] |(is_right?0x1:0x0);
////        data_send[7]=freq>>24;
////        data_send[8]=(freq>>16)&0xff;
////        data_send[9]=(freq>>8)&0xff;
////        data_send[10]=(freq)&0xff;
////开关音量
////    uint32_t sleep_time=1000;
////    bool left_open=true;
////    bool right_open=false;
////    uint8_t data_send[]={0x55,0xaa,0x08,0x05,0x81,0xf3,0x50,0x00,0x00,0x02,0x00,0xa4,0x00};
////    data_send[7]=left_open?0x1:0x0;
////    data_send[8]=right_open?0x1:0x0;


//    uint32_t length=sizeof (data_send)-1;
//    printf("%u \r\n",length);
//    const uint8_t *p_src = data_send;
//    uint8_t check_code = 0;
//    for (uint64_t i = 2; i < length; ++i)
//        check_code ^= p_src[i];
//    data_send[length]=check_code;

//    while(1)
//    {
//        printf("send:");
//        for(uint32_t i=0;i<length+1;++i)
//        {
//            printf("%02X ",data_send[i]);
//        }
//        printf("\r\n");
//        write(fd,data_send,length+1);
//        usleep(sleep_time*1000);
//    }
//    return 0;
//}
int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    //aimy::AimyLogger::Instance().set_log_path("/userdata/aimy/logs/test","test");
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    if(!AIMY_UTILS::acquireSigleInstanceLcok())
    {
        AIMY_ERROR("is running exit!");
        return -1;
    }
    auto path=getenv("tty_name");
    int fd=-1;
    if(path)
    {
        fd=aserial_open(path);
    }
    else {
        fd=aserial_open("/dev/ttyUSB0");
    }
    aserial_set_opt(fd,115200,8,'N',1);
    std::thread t([&](){
        uint8_t buffer[256];
        while(1)
        {
            memset(buffer,0,256);
            auto ret=read(fd,buffer,256);
            if(ret>0)
            {
                printf("recv:");
                for(auto i=0;i<ret;++i)
                {
                    printf("%02X ",buffer[i]);
                }
                printf("\r\n");
            }

        }
    });
    t.detach();
    uint32_t length=0;
    uint8_t *data_src=nullptr;
    uint32_t sleep_time=10;
    uint32_t option=std::stoi(argv[1]);
    switch (option) {
    case 0:{
        //获取模块的通道
        sleep_time=10;
        uint8_t data_send[]={0x0B,0x55,0xaa,0x08,0x03,0x81,0xf3,0x01,0x02,0x00,0xa4,0x00};
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 1:{
        // 1.获得版本信息
        sleep_time=10;
        uint8_t data_send[]={0x0B,0x55,0xaa,0x08,0x03,0x81,0xf3,0x93,0x02,0x00,0xa4,0x00};
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 2:{
        //3.获取模块的信号强度
        sleep_time=2000;
        uint8_t data_send[]={0x0B,0x55,0xaa,0x08,0x03,0x81,0xf3,0x02,0x02,0x00,0xa4,0x00};
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 3:{
        //4.配对
        sleep_time=5000;
        bool pair_left=true;
        bool pair_right=true;

        uint8_t data_send[]={0x0D,0x55,0xaa,0x08,0x05,0x81,0xf3,0x03,0x00,0x01,0x02,0x00,0xa4,0x00};
        data_send[8]=pair_left?0x01:0x00;
        data_send[9]=pair_right?0x01:0x00;
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 4:{
        //频道扫描
        sleep_time=15000;
        bool  is_right=false;
        uint8_t data_send[]={0x0D,0x55,0xaa,0x08,0x05,0x81,0xf3,0x10,0x01,0x96,0x02,0x00,0xa4,0x00};
        data_send[7]=data_send[7] |(is_right?0x1:0x0);
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 5:{
        //    //设置频道
        sleep_time=15000;
        bool  is_right=false;
        uint8_t channel=0x0A;
        uint8_t data_send[]={0x10,0x55,0xaa,0x08,0x08,0x81,0xf3,0x20,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0xa4,0x00};
        data_send[7]=data_send[7] |(is_right?0x1:0x0);
        data_send[8]=channel;
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 6:{
        //搜索空闲频道
        sleep_time=15000;
        bool  is_right=false;
        uint8_t start_channel=1;
        uint8_t end_channel=150;
        uint8_t data_send[]={0x0f,0x55,0xaa,0x08,0x07,0x81,0xf3,0x30,0x00,0x00,0x0C,0x26,0x02,0x00,0xa4,0x00};
        data_send[7]=data_send[7] |(is_right?0x1:0x0);
        data_send[8]=start_channel;
        data_send[9]=end_channel;
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 7:{
        //设置发送频率
        sleep_time=1000;
        bool  is_right=false;
        uint32_t freq=600000;
        uint8_t data_send[]={0x0f,0x55,0xaa,0x08,0x07,0x81,0xf3,0x40,0x00,0x00,0x0C,0x26,0x02,0x00,0xa4,0x00};
        data_send[7]=data_send[7] |(is_right?0x1:0x0);
        data_send[8]=freq>>24;
        data_send[9]=(freq>>16)&0xff;
        data_send[10]=(freq>>8)&0xff;
        data_send[11]=(freq)&0xff;
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 8:{
        //开关音量
        sleep_time=1000;
        bool left_open=true;
        bool right_open=false;
        uint8_t data_send[]={0x0d,0x55,0xaa,0x08,0x05,0x81,0xf3,0x50,0x00,0x00,0x02,0x00,0xa4,0x00};
        data_send[8]=left_open?0x1:0x0;
        data_send[9]=right_open?0x1:0x0;
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    case 9:{
        //查询版本
        sleep_time=10;
        uint8_t data_send[]={0x06,0x55,0xaa,0x80,0x01,0xa1,0x20};
        length=sizeof (data_send)-1;
        data_src=data_send;
        printf("send:");
        for(uint32_t i=0;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
    }
        break;
    default:
        printf("0: get module channel info\r\n");
        printf("1: get module version info\r\n");
        printf("2: get module signal strength info\r\n");
        printf("3: pair\r\n");
        printf("4: scan\r\n");
        printf("5: set left channel to 10 \r\n");
        printf("6: search a useful channel for left channel \r\n");
        printf("7: set left channel freq to 600k[khz] \r\n");
        printf("8: set volume left [open] right [close] \r\n");
        printf("9: query dsp version \r\n");
        return 0;
    }
    if(argc>=3)sleep_time=std::stoi(argv[2]);
    printf("%u \r\n",length);
    const uint8_t *p_src = data_src;
    uint8_t check_code = 0;
    for (uint64_t i = 3; i < length; ++i)
        check_code ^= p_src[i];
    data_src[length]=check_code;
    while(1)
    {
        printf("send:");
        for(uint32_t i=1;i<length+1;++i)
        {
            printf("%02X ",data_src[i]);
        }
        printf("\r\n");
        write(fd,data_src+1,length+1-1);
        usleep(sleep_time*1000);
    }
    return 0;
}
