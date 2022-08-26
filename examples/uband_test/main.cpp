#include "log/aimy-log.h"
#include "core/core-include.h"
#include "third_party/json/cjson-interface.h"
#include "imp/utils/common_utils.h"
#include "imp/common/serial_utils.h"
#include "third_party/json/cjson-interface.h"
using namespace aimy;
using neb::CJsonObject;

static void print_help_info(const char * proname)
{
    fprintf(stderr,"Usage:%s <test_config_file_path> \t  default value:is ./test.conf \r\n",proname);
    fprintf(stderr,"有疑问请联系:469953258@qq.com \r\n ");
    fprintf(stderr,"\t -----------------------main config----------------------- \r\n");
    fprintf(stderr,"\t \t dev_path \t 通讯所用的hid设备或串口路径 如1./dev/ttyUSB0 2./dev/hidraw0\r\n");
    fprintf(stderr,"\t \t bandrate \t 通讯所用的波特率\r\n");
    fprintf(stderr,"\t \t dev_addr \t 通讯所用的设备地址,241外置1 242外置2 243内置接收板\r\n");
    fprintf(stderr,"\t \t left_channel \t 预设的左通道频道值 [1-150] \r\n");
    fprintf(stderr,"\t \t right_channel \t 预设的右通道频道值 [1-150] \r\n");
    fprintf(stderr,"\t \t debug_level \t log等级 0:debug 1:info 2:warrning 3:error 4: fatalerror [0-6]\r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------item common config----------------------- \r\n");
    fprintf(stderr,"\t \t interval \t 一次有效数据接收的最长等待时间 \r\n");
    fprintf(stderr,"\t \t test_cnt \t 测试命令发送次数 \r\n");
    fprintf(stderr,"\t \t target_cnt \t 达标次数，接收到的有效数据数量小于此值判断为检测失败 \r\n");
    fprintf(stderr,"\t \t next_item_test_wait_msec \t 上一个测试项完成与下一个测试项开始的等待时间间隔 \r\n");
    fprintf(stderr,"\t -----------------------set_channel_config----------------------- \r\n");
    fprintf(stderr,"\t \t 有效数据:查询到的通道与预设的通道值一致 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------get_module_config----------------------- \r\n");
    fprintf(stderr,"\t \t 有效数据:查询到的通道与预设的通道值一致 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------set_volume_config----------------------- \r\n");
    fprintf(stderr,"\t \t 有效数据：设置过后，回读声音状态与上次设置的一致 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------pair_config----------------------- \r\n");
    fprintf(stderr,"\t \t pair_left \t 配对左通道 true代表进行配对 false代表不进行配对 \r\n");
    fprintf(stderr,"\t \t pair_right \t 配对右通道 true代表进行配对 false代表不进行配对 \r\n");
    fprintf(stderr,"\t \t 有效数据： 需要配对通道配对成功，且配对通道一致 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------get_version_config----------------------- \r\n");
    fprintf(stderr,"\t \t minor_version \t 当前期望的版本，若获取到的版本不一致，测试不通过 \r\n");
    fprintf(stderr,"\t \t left_signal \t 期望左通道的U段数据信号状态[true:有信号 false:无信号]，若状态不一致，测试不通过 \r\n");
    fprintf(stderr,"\t \t right_signal \t 期望右通道的U段数据信号状态[true:有信号 false:无信号]，若状态不一致，测试不通过 \r\n");
    fprintf(stderr,"\t \t 有效数据：版本一致，信号状态一致 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------scan_channel_config----------------------- \r\n");
    fprintf(stderr,"\t \t scan_left \t 是否扫描左通道 true:左 false:右 \r\n");
    fprintf(stderr,"\t \t 有效数据:搜索到频道数目不为0 \r\n");
    fprintf(stderr,"\r\n");
    fprintf(stderr,"\t -----------------------scan_free_channel_config----------------------- \r\n");
    fprintf(stderr,"\t \t start_channel \t 扫描的频道起始值 [1-end_channel] \r\n");
    fprintf(stderr,"\t \t end_channel \t 扫描的频道结束值 [start_channel-150] \r\n");
    fprintf(stderr,"\t \t rssi_threshold \t 信号强度阈值，推荐值12，实测经验值25  [>=12] \r\n");
    fprintf(stderr,"\t \t snr_threshold \t 信噪比阈值，推荐值38，实测经验值100  [>=20] \r\n");
    fprintf(stderr,"\t \t scan_left \t 是否扫描左通道 true:左 false:右 \r\n");
    fprintf(stderr,"\t \t 有效数据:搜索到处于[1-150]的通讯频道 \r\n");
    fprintf(stderr,"\r\n");

}

int main(int argc,char *argv[])
{
    (void)argc;
    (void)argv;
    aimy::AimyLogger::Instance().register_handle();
    aimy::AimyLogger::Instance().set_log_to_std(true);
    atexit([](){
        aimy::AimyLogger::Instance().unregister_handle();
    });
    std::string config_path="./test.conf";
    std::string option;
    if(argc==1)
    {
        print_help_info(argv[0]);
    }
    else {
        option=argv[1];
    }
    if(option=="-h"||option=="--help"||option=="help")
    {
        print_help_info(argv[0]);
        _Exit(-1);
    }
    else if(!option.empty()) {
        config_path=option;
    }
    CJsonObject save_object;
    CJsonObject *load_object=CJsonObject::CreateInstance(config_path);


    int fd=-1;
    uint8_t dev_addr=0xf3;
    int data_offset=0;
    uint64_t left_channel=0x0A;//10
    uint64_t right_channel=0x64;//96
    bool test_success=true;
    int debug_level=0;
    {//init
        std::string dev_path="/dev/hidraw0";
        load_object->Get("dev_path",dev_path);
        save_object.Add("dev_path",dev_path);
        fd=aserial_open(dev_path.c_str());

        uint32_t bandrate=115200;
        load_object->Get("bandrate",bandrate);
        save_object.Add("bandrate",bandrate);
        aserial_set_opt(fd,bandrate,8,'N',1);

        uint32_t dev_addr_temp=dev_addr;
        load_object->Get("dev_addr",dev_addr_temp);
        save_object.Add("dev_addr",dev_addr_temp);
        dev_addr=dev_addr_temp&0xff;

        load_object->Get("data_offset",data_offset);
        save_object.Add("data_offset",data_offset);

        load_object->Get("data_offset",data_offset);
        save_object.Add("data_offset",data_offset);

        load_object->Get("left_channel",left_channel);
        save_object.Add("left_channel",left_channel);

        load_object->Get("right_channel",right_channel);
        save_object.Add("right_channel",right_channel);

        load_object->Get("debug_level",debug_level);
        save_object.Add("debug_level",debug_level);

        aimy::AimyLogger::Instance().set_minimum_log_level(debug_level);
        AIMY_DEBUG("global_config->dev_path:%s bandrate:%d dev_addr:%02x data_offset:%d left_channel:%d right_channel:%d",dev_path.c_str(),bandrate,dev_addr,data_offset
                   ,left_channel,right_channel);
    }
    uint8_t control_bit=0x04;
    control_bit=0x08;
    AIMY_BACKTRACE("start uband test");
    uint64_t next_item_test_wait_msec=100;
    uint8_t recv_buf[256]={0};
    {
        AIMY_DEBUG("------------------------set channel start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("set_channel_config",submodue);

        int interval=2000;
        int test_cnt=2;
        int target_cnt=2;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);

            AIMY_DEBUG("set_channel_config->interval[%d] test_cnt[%d] target_cnt[%d]",interval,test_cnt
                       ,target_cnt);
        }

        uint8_t data_send[]={0x0D,0x55,0xaa,control_bit,0x08,0x81,0xf3,0x20,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0xa4,0x00};
        data_send[6]=dev_addr;
        int recv_cnt=0;
        int recv_left_channel=0;
        int recv_right_channel=0;

        while(test_cnt-->0)
        {
            bool  is_right=test_cnt%2;
            data_send[7]=0x20 |(is_right?0x1:0x0);
            if(is_right)
            {
                data_send[8]=right_channel;
            }
            else {
                data_send[8]=left_channel;
            }
            uint32_t length=sizeof (data_send)-1;
            const uint8_t *p_src = data_send;
            uint8_t check_code = 0;
            for (uint64_t i = 3; i < length; ++i)
                check_code ^= p_src[i];
            data_send[length]=check_code;
            if(test_cnt<=1)AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=16;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x08&&recv_buf[offset+5]==dev_addr&&(recv_buf[offset+6]&0x20))break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    find=true;
                    if(recv_buf[offset+6]==0x20)
                    {
                        recv_left_channel=recv_buf[offset+7];
                    }
                    else {
                        recv_right_channel=recv_buf[offset+7];
                    }
                    if(recv_left_channel!=left_channel)
                    {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                            AIMY_WARNNING("left channel is not match [%u->%u]",recv_left_channel,left_channel);
                        }

                    }
                    if(recv_right_channel!=right_channel)
                    {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                            AIMY_WARNNING("right channel is not match [%u->%u]",recv_right_channel,right_channel);
                        }
                    }
                    recv_cnt++;
                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }

        save_object.Add("set_channel_config",save_submodule);
        if(target_cnt>recv_cnt||left_channel!=recv_left_channel||right_channel!=recv_right_channel)
        {
            test_success=false;
            AIMY_ERROR("result[failed][set_channel]:[cnt %d->%d] [left %d->%d] [right %d->%d] ",target_cnt,recv_cnt,left_channel,recv_left_channel,right_channel,recv_right_channel);
        }
        else {
            AIMY_INFO("result[success][set_channel]:[cnt %d->%d] [left %d->%d] [right %d->%d] ",target_cnt,recv_cnt,left_channel,recv_left_channel,right_channel,recv_right_channel);
        }
        AIMY_DEBUG("------------------------set channel  end------------------------------");

    }
    usleep(next_item_test_wait_msec*1000);
    next_item_test_wait_msec=100;
    {
        AIMY_DEBUG("------------------------get module start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("get_module_config",submodue);

        int interval=100;
        int test_cnt=100;
        int target_cnt=100;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);

            AIMY_DEBUG("module_config->interval[%d] test_cnt[%d] target_cnt[%d]",interval,test_cnt
                       ,target_cnt);
        }
        uint8_t data_send[]={0x0B,0x55,0xaa,control_bit,0x03,0x81,0xf3,0x01,0x02,0x00,0xa4,0x00};
        data_send[6]=dev_addr;
        uint32_t length=sizeof (data_send)-1;
        const uint8_t *p_src = data_send;
        uint8_t check_code = 0;
        for (uint64_t i = 3; i < length; ++i)
            check_code ^= p_src[i];
        data_send[length]=check_code;

        AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());

        int recv_cnt=0;
        int recv_left_channel=0;
        int recv_right_channel=0;

        while(test_cnt-->0)
        {
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=14;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x06&&recv_buf[offset+5]==dev_addr&&recv_buf[offset+6]==0x01)break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    find=true;
                    recv_left_channel=recv_buf[offset+7];
                    recv_right_channel=recv_buf[offset+8];
                    if((recv_left_channel+1)!=left_channel)
                    {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                            AIMY_WARNNING("left channel is not match [%u->%u]",recv_left_channel+1,left_channel);
                        }

                    }
                    if((recv_right_channel+1)!=right_channel)
                    {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                            AIMY_WARNNING("right channel is not match [%u->%u]",recv_right_channel+1,right_channel);
                        }
                    }
                    recv_cnt++;
                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }

        save_object.Add("get_module_config",save_submodule);
        if((target_cnt>recv_cnt||left_channel!=(recv_left_channel+1)||right_channel!=(recv_right_channel+1))&&target_cnt!=0)
        {
            test_success=false;
            AIMY_ERROR("result[failed][get module]:[cnt %d->%d] [left %d->%d] [right %d->%d] ",target_cnt,recv_cnt,left_channel,recv_left_channel+1,right_channel,recv_right_channel+1);
        }
        else {
            AIMY_INFO("result[success][get module]:[cnt %d->%d] [left %d->%d] [right %d->%d] ",target_cnt,recv_cnt,left_channel,recv_left_channel+1,right_channel,recv_right_channel+1);
        }
        AIMY_DEBUG("------------------------get module end------------------------------");

    }
    usleep(next_item_test_wait_msec*1000);
    next_item_test_wait_msec=100;
    {
        AIMY_DEBUG("------------------------set volume start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("set_volume_config",submodue);

        int interval=100;
        int test_cnt=2;
        int target_cnt=2;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);

            AIMY_DEBUG("set_volume_config->interval[%d] test_cnt[%d] target_cnt[%d]",interval,test_cnt
                       ,target_cnt);
        }

        uint8_t data_send[]={0x0d,0x55,0xaa,control_bit,0x05,0x81,0xf3,0x50,0x00,0x00,0x02,0x00,0xa4,0x00};
        data_send[6]=dev_addr;
        int recv_cnt=0;
        test_cnt=test_cnt*4;
        while(test_cnt-->0)
        {
            int status=test_cnt%4;
            if(status==3)
            {
                data_send[8]=0x00;
                data_send[9]=0x00;
            }
            else if(status==2||status==0){
                data_send[8]=0x02;
                data_send[9]=0x02;
            }
            else {
                data_send[8]=0x01;
                data_send[9]=0x01;
            }
            uint32_t length=sizeof (data_send)-1;
            const uint8_t *p_src = data_send;
            uint8_t check_code = 0;
            for (uint64_t i = 3; i < length; ++i)
                check_code ^= p_src[i];
            data_send[length]=check_code;
            if(test_cnt<=3)AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=13;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x05&&recv_buf[offset+5]==dev_addr&&recv_buf[offset+6]==0x50)break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    find=true;
                    if(status==2&&(recv_buf[offset+7]==0x00&&recv_buf[offset+8]==0x00))
                    {
                        recv_cnt++;
                    }
                    else if (status==0&&(recv_buf[offset+7]==0x01&&recv_buf[offset+8]==0x01)) {
                        recv_cnt++;
                    }
                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }
        recv_cnt=recv_cnt/2;
        save_object.Add("set_volume_config",save_submodule);
        if(target_cnt>recv_cnt)
        {
            test_success=false;
            AIMY_ERROR("result[failed][set_volume]:[cnt %d->%d]",target_cnt,recv_cnt);
        }
        else {
            AIMY_INFO("result[success][set_volume]:[cnt %d->%d] ",target_cnt,recv_cnt,left_channel);
        }
        AIMY_DEBUG("------------------------set volume  end------------------------------");

    }
    usleep(next_item_test_wait_msec*1000);
    next_item_test_wait_msec=2000;
    {
        AIMY_DEBUG("------------------------pair start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("pair_config",submodue);

        int interval=20000;
        int test_cnt=2;
        int target_cnt=1;
        bool pair_left=true;
        bool pair_right=false;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("pair_left",pair_left);
            save_submodule.Add("pair_left",pair_left,pair_left);

            submodue.Get("pair_right",pair_right);
            save_submodule.Add("pair_right",pair_right,pair_right);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);

            AIMY_DEBUG("pair_config->interval[%d] test_cnt[%d] target_cnt[%d] pair_left[%d] pair_right[%d]",interval,test_cnt
                       ,target_cnt,pair_left,pair_right);
        }

        uint8_t data_send[]={0x0D,0x55,0xaa,control_bit,0x05,0x81,0xf3,0x03,0x00,0x01,0x02,0x00,0xa4,0x00};

        data_send[6]=dev_addr;
        int recv_cnt=0;
        while(test_cnt-->0)
        {
            data_send[8]=pair_left?0x01:0x00;
            data_send[9]=pair_right?0x01:0x00;
            uint32_t length=sizeof (data_send)-1;
            const uint8_t *p_src = data_send;
            uint8_t check_code = 0;
            for (uint64_t i = 3; i < length; ++i)
                check_code ^= p_src[i];
            data_send[length]=check_code;
            if(test_cnt<=3)AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=14;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x06&&recv_buf[offset+5]==dev_addr&&recv_buf[offset+6]==0x01)break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    find=true;
                    uint8_t recv_left_channel=recv_buf[offset+7];
                    uint8_t recv_right_channel=recv_buf[offset+8];
                    bool left_success=recv_buf[offset+9]&0x10;
                    bool right_success=recv_buf[offset+9]&0x1;
                    if(pair_left)
                    {
                        if(left_success&&left_channel==recv_left_channel+1)recv_cnt++;
                        else {
                            static bool is_print=false;
                            if(!is_print)
                            {
                                is_print=true;
                                AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                                AIMY_WARNNING("pair left is failed status[%d] need_channel[%d] actual[%d]",left_success,left_channel,recv_left_channel+1);
                            }
                        }
                    }
                    if(pair_right)
                    {
                        if(pair_right&&right_success&&right_channel==recv_right_channel+1)recv_cnt++;
                        else {
                            static bool is_print=false;
                            if(!is_print)
                            {
                                is_print=true;
                                AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                                AIMY_WARNNING("pair right is failed status[%d] need_channel[%d] actual[%d]",right_success,right_channel,recv_right_channel+1);
                            }
                        }
                    }

                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }
        save_object.Add("pair_config",save_submodule);
        if(target_cnt>recv_cnt)
        {
            test_success=false;
            AIMY_ERROR("result[failed][pair:[cnt %d->%d]",target_cnt,recv_cnt);
        }
        else {
            AIMY_INFO("result[success][pair]:[cnt %d->%d] ",target_cnt,recv_cnt,left_channel);
        }
        AIMY_DEBUG("------------------------pair  end------------------------------");

    }
    usleep(next_item_test_wait_msec*1000);
    next_item_test_wait_msec=100;
    {
        AIMY_DEBUG("------------------------get version start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("get_version_config",submodue);

        int interval=1000;
        int test_cnt=10;
        int target_cnt=1;
        uint64_t minor_version=0x22;
        bool left_signal=true;
        bool right_signal=false;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("minor_version",minor_version);
            save_submodule.Add("minor_version",minor_version);

            submodue.Get("left_signal",left_signal);
            save_submodule.Add("left_signal",left_signal,left_signal);

            submodue.Get("right_signal",right_signal);
            save_submodule.Add("right_signal",right_signal,right_signal);
            AIMY_BACKTRACE("get_version_config->interval[%d] test_cnt[%d] target_cnt[%d] minor_version[0x%02x] left_signal[%d] right_signal[%d]",interval,test_cnt
                       ,target_cnt,minor_version,left_signal,right_signal);
        }
        uint8_t data_send[]={0x0B,0x55,0xaa,control_bit,0x03,0x81,0xf3,0x93,0x02,0x00,0xa4,0x00};
        data_send[6]=dev_addr;
        uint32_t length=sizeof (data_send)-1;
        const uint8_t *p_src = data_send;
        uint8_t check_code = 0;
        for (uint64_t i = 3; i < length; ++i)
            check_code ^= p_src[i];
        data_send[length]=check_code;

        AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());

        int recv_cnt=0;
        uint8_t version=0;
        bool left_has_signal=false;
         bool right_has_signal=false;
        while(test_cnt-->0)
        {
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=15;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x07&&recv_buf[offset+5]==dev_addr&&recv_buf[offset+6]==0x93)break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    find=true;
                    version=recv_buf[offset+7];
                    left_has_signal=recv_buf[offset+8]==0x01;
                    right_has_signal=recv_buf[offset+9]==0x01;
                    if(version<minor_version||(left_has_signal!=left_signal)||(right_has_signal!=right_signal))
                    {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("recv failed message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                            AIMY_WARNNING("version status is not match version[need:%02x->actual:%02x] signal_status[left:need:%d->%d] [right:need:%d->%d]",minor_version,version
                                          ,left_signal,left_has_signal
                                          ,right_signal,right_has_signal);
                        }

                    }
                    recv_cnt++;
                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }

        save_object.Add("get_version_config",save_submodule);
        if(target_cnt>recv_cnt||version!=minor_version||(left_has_signal!=left_signal)||(right_has_signal!=right_signal))
        {
            test_success=false;
            AIMY_ERROR("result[failed][get_version]:cnt[%d->%d] version[need:%02x->actual:%02x] signal_status[left:need:%d->%d] [right:need:%d->%d]",target_cnt,recv_cnt,minor_version,version
                       ,left_signal,left_has_signal
                       ,right_signal,right_has_signal);
        }
        else {
            AIMY_INFO("result[success][get_version]:cnt[%d->%d] version[need:%02x->actual:%02x] signal_status[left:need:%d->%d] [right:need:%d->%d]",target_cnt,recv_cnt,minor_version,version
                      ,left_signal,left_has_signal
                      ,right_signal,right_has_signal);
        }
        AIMY_DEBUG("------------------------get version end------------------------------");

    }
    {
        AIMY_DEBUG("------------------------scan channel start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("scan_channel_config",submodue);

        int interval=10000;
        int test_cnt=1;
        int target_cnt=1;
        bool scan_left=true;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("scan_left",scan_left);
            save_submodule.Add("scan_left",scan_left,scan_left);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);

            AIMY_DEBUG("scan_channel_config->interval[%d] test_cnt[%d] target_cnt[%d] scan_left[%d]",interval,test_cnt
                       ,target_cnt,scan_left);
        }
        uint8_t data_send[]={0x0D,0x55,0xaa,control_bit,0x05,0x81,0xf3,0x10,0x01,0x96,0x02,0x00,0xa4,0x00};
        data_send[7]=0x10 |(scan_left?0x0:0x1);
        data_send[6]=dev_addr;
        uint32_t length=sizeof (data_send)-1;
        const uint8_t *p_src = data_send;
        uint8_t check_code = 0;
        for (uint64_t i = 3; i < length; ++i)
            check_code ^= p_src[i];
        data_send[length]=check_code;

        AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());

        int recv_cnt=0;
        while(test_cnt-->0)
        {
            write(fd,data_send+data_offset,length+1-data_offset);
            int false_cnt=0;
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=8;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]>0&&recv_buf[offset+5]==dev_addr&&(recv_buf[offset+6]==0x10||recv_buf[offset+6]==0x11))break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    AIMY_DEBUG("scan result:%s",AimyLogger::formatHexToString(recv_buf+offset,recv_len-offset).c_str());
                    find=true;
                    recv_cnt++;
                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    false_cnt++;
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                    if(false_cnt>5)break;
                }
            }
        }

        save_object.Add("scan_channel_config",save_submodule);
        if(target_cnt>recv_cnt)
        {
            test_success=false;
            AIMY_ERROR("result[failed][scan_channel]:cnt[%d->%d]",target_cnt,recv_cnt);
        }
        else {
            AIMY_INFO("result[success][scan_channel]:cnt[%d->%d]",target_cnt,recv_cnt);
        }
        AIMY_DEBUG("------------------------scan channel end------------------------------");

    }
    usleep(next_item_test_wait_msec*1000);
    next_item_test_wait_msec=100;
    {
        AIMY_DEBUG("------------------------scan free channel start------------------------------");
        //获取模块的通道
        CJsonObject submodue;
        CJsonObject save_submodule;
        load_object->Get("scan_free_channel_config",submodue);

        int interval=10000;
        int test_cnt=1;
        int target_cnt=1;
        bool scan_left=true;
        uint64_t start_channel=1;
        uint64_t end_channel=150;
        uint64_t rssi_threshold=25;
        uint64_t snr_threshold=100;
        {
            //load config
            submodue.Get("interval",interval);
            save_submodule.Add("interval",interval);

            submodue.Get("test_cnt",test_cnt);
            save_submodule.Add("test_cnt",test_cnt);

            submodue.Get("target_cnt",target_cnt);
            save_submodule.Add("target_cnt",target_cnt);

            submodue.Get("start_channel",start_channel);
            save_submodule.Add("start_channel",start_channel);

            submodue.Get("end_channel",end_channel);
            save_submodule.Add("end_channel",end_channel);


            submodue.Get("rssi_threshold",rssi_threshold);
            save_submodule.Add("rssi_threshold",rssi_threshold);

            submodue.Get("snr_threshold",snr_threshold);
            save_submodule.Add("snr_threshold",snr_threshold);

            submodue.Get("scan_left",scan_left);
            save_submodule.Add("scan_left",scan_left,scan_left);

            submodue.Get("next_item_test_wait_msec",next_item_test_wait_msec);
            save_submodule.Add("next_item_test_wait_msec",next_item_test_wait_msec);


            AIMY_DEBUG("scan_free_channel_config->interval[%d] test_cnt[%d] target_cnt[%d] scan_left[%d] start_channel[%d] end_channel[%d]",interval,test_cnt
                       ,target_cnt,scan_left,start_channel,end_channel);
        }
        uint8_t data_send[]={0x0f,0x55,0xaa,control_bit,0x07,0x81,0xf3,0x30,0x00,0x00,0x0C,0x26,0x02,0x00,0xa4,0x00};
        data_send[7]=0x30 |(scan_left?0x0:0x1);
        data_send[8]=start_channel;
        data_send[9]=end_channel;
        data_send[10]=rssi_threshold;
        data_send[11]=snr_threshold;
        data_send[6]=dev_addr;
        uint32_t length=sizeof (data_send)-1;
        const uint8_t *p_src = data_send;
        uint8_t check_code = 0;
        for (uint64_t i = 3; i < length; ++i)
            check_code ^= p_src[i];
        data_send[length]=check_code;

        AIMY_DEBUG("send %s",AimyLogger::formatHexToString(data_send+data_offset,length+1-data_offset).c_str());

        int recv_cnt=0;
        uint8_t scan_channel=0x00;
        while(test_cnt-->0)
        {
            write(fd,data_send+data_offset,length+1-data_offset);
            while(1)
            {
                memset(recv_buf,0,256);
                int recv_len=aserial_read(fd,recv_buf,256,interval);
                if(recv_len<=0)break;
                bool find=false;
                uint32_t offset=0;
                uint32_t response_min_size=8;
                while(offset+response_min_size<=recv_len)
                {
                    while(offset+response_min_size<=recv_len)
                    {
                        if(recv_buf[offset]==0x55&&recv_buf[offset+1]==0xAA&&recv_buf[offset+3]==0x06&&recv_buf[offset+5]==dev_addr&&(recv_buf[offset+6]==data_send[7]))break;
                        ++offset;
                    }
                    if(offset+response_min_size>recv_len)break;
                    AIMY_DEBUG("scan result:%s",AimyLogger::formatHexToString(recv_buf+offset,recv_len-offset).c_str());
                    find=true;
                    scan_channel=recv_buf[offset+7];
                    if(scan_channel>=0&&scan_channel<=150)
                    {
                        recv_cnt++;
                    }
                    else {
                        static bool is_print=false;
                        if(!is_print)
                        {
                            is_print=true;
                            AIMY_WARNNING("scan channel failed is [channel:0x%02x] buffer:%s",scan_channel,AimyLogger::formatHexToString(recv_buf+offset,recv_len-offset).c_str());
                        }
                    }

                    offset+=response_min_size;
                }
                if(find)break;
                else {
                    AIMY_DEBUG("recv false message:%s",AimyLogger::formatHexToString(recv_buf,recv_len).c_str());
                }
            }
        }

        save_object.Add("scan_free_channel_config",save_submodule);
        if(target_cnt>recv_cnt)
        {
            test_success=false;
            AIMY_ERROR("result[failed][scan_free_channel]:cnt[%d->%d] scan_channel[0x%02x]",target_cnt,recv_cnt,scan_channel);
        }
        else {
            AIMY_INFO("result[success][scan_free_channel]:cnt[%d->%d] scan_channel[0x%02x]",target_cnt,recv_cnt,scan_channel);
        }
        AIMY_DEBUG("------------------------scan free channel end------------------------------");

    }
    AIMY_BACKTRACE("stop uband test status[%s]",test_success?"success":"failed");
    save_object.SetSavePath(config_path);
    save_object.SaveToFile();
    return test_success?0:1;
    return 0;
}
