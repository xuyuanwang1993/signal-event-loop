#include "bluez-test-task.h"
#include<atomic>
#include<algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace aimy;
static std::atomic<int> recv_cnt{0};
static BluetoothTest * g_handle=nullptr;
static int rssi=-200;//信号强度
static uint8_t hex_to_uint8(const std::string &str,uint8_t convert_offset=0)
{
    uint8_t ret=0;
    int base=1;
    auto iter=str.rbegin()+convert_offset;
    for(int i=0;i<2&&iter!=str.rend();++iter,++i)
    {
        char c=*iter;
        if(c>='0'&&c<='9')
        {
            ret+=(c-'0')*base;
        }
        else if (c>='a'&&c<='f') {
            ret+=(c-'a'+10)*base;
        }
        else {
            throw std::invalid_argument("bad mac format");
        }
        base*=16;
    }
    return ret;
}
static std::string str_to_BDADDR(const std::string &mac)
{
    uint8_t bd_save[6];
    memset(bd_save,0,6);
    std::string part1;
    std::string part2;
    std::string part3;
    auto pos1=mac.find(':');
    if(pos1==std::string::npos)return mac;
    part1=mac.substr(0,pos1);
    auto pos2=mac.find(':',pos1+1);
    if(pos2==std::string::npos)return mac;
    auto len=pos2-pos1-1;
    if(len>0)
    {
        part2=mac.substr(pos1+1,len);
    }
    part3=mac.substr(pos2+1);
    std::transform(part1.begin(),part1.end(),part1.begin(),::tolower);
    std::transform(part2.begin(),part2.end(),part2.begin(),::tolower);
    std::transform(part3.begin(),part3.end(),part3.begin(),::tolower);
    if(part1.size()>2)
    {
        bd_save[0]=hex_to_uint8(part1,2);
    }
    if(part1.size()>0)
    {
        bd_save[1]=hex_to_uint8(part1);
    }
    if(part2.size()>0)
    {
        bd_save[2]=hex_to_uint8(part2);
    }
    if(part3.size()>4)
    {
        bd_save[3]=hex_to_uint8(part3,4);
    }
    if(part3.size()>2)
    {
        bd_save[4]=hex_to_uint8(part3,2);
    }
    if(part3.size()>0)
    {
        bd_save[5]=hex_to_uint8(part3);
    }
    char ret[20];
    memset(ret,0,20);
    sprintf(ret,"%02X:%02X:%02X:%02X:%02X:%02X",bd_save[0],bd_save[1],bd_save[2],bd_save[3],bd_save[4],bd_save[5]);
    return ret;
}
BluetoothTest::BluetoothTest(Object *parent,int64_t timeoutMsec):Object(parent),finshTest(this)
{
    timeoutTimer=this->addTimer(timeoutMsec);
    timeoutTimer->setSingle(true);
    timeoutTimer->timeout.connect(this,std::bind(&BluetoothTest::on_timeout,this));
}

BluetoothTest::~BluetoothTest()
{

}

void BluetoothTest::on_recv_mac(std::string mac,std::string pswd)
{
    recv_cnt=0;
    g_handle=this;
    convertMac=str_to_BDADDR(mac);
    AIMY_MARK("test_mac[%s] pswd[%s]",convertMac.c_str(),pswd.c_str());

    if(!initBluetoothConnection())handle_result(-1,"bluetooth connection init failed!");
    on_setup();
}

void BluetoothTest::start()
{

    timeoutTimer->start();
    isrunning.exchange(true);
    AIMY_INFO("start---------%d",isrunning.load());
}

void BluetoothTest::stop()
{
    isrunning.exchange(false);
    AIMY_INFO("stop---------%d",isrunning.load());
    timeoutTimer->stop();
    if(recv_cnt>=2)
    {
        handle_result(0);
    }
    else {
        handle_result(1,"just recv "+std::to_string(recv_cnt)+" packets,need 2!");
    }
}

int BluetoothTest::result()const
{
    return recv_cnt>=2?0:-1;
}

bool BluetoothTest::initBluetoothConnection()
{
    releaseBluetoothConnection();
    system("mknod /dev/rfcomm100 c 216 100");
    usleep(500);
    system((std::string("rfcomm bind 100 ")+convertMac).c_str());
    usleep(500);
    system((std::string("rfcomm connect 100 ")+convertMac).c_str());
    sleep(5);
    return true;
}



void BluetoothTest::releaseBluetoothConnection()
{
    system("rfcomm release 100");
    system("rm /dev/rfcomm100");
}

void BluetoothTest::handle_result(int result, const std::string &result_sdp)
{
    printf("please provide a rssi_limit\r\n");
    AIMY_INFO("##BT_RSSI=%d##",rssi);
    AIMY_INFO("##BT_Result=%s##",result_sdp.c_str());
    timeoutTimer->release();
    releaseBluetoothConnection();
    isrunning.exchange(false);
    finshTest.emit(result);

}

void BluetoothTest::on_timeout()
{
    if(!isrunning)return;
    AIMY_WARNNING("timeout");
    if(recv_cnt<=0)handle_result(-2,"timeout for check");
    else {
        handle_result(0);
    }
}

void BluetoothTest::on_setup()
{
    int send_cnt=10;
    usleep(500);
    auto fd=open("/dev/rfcomm100",O_RDWR);
    if(fd<0)
    {
        handle_result(1,std::string("open rfcomm100 failed!")+strerror(errno));
        return;
    }
    std::thread recv_thread([=](){
        char buf[7]={0};
        while(isrunning&&recv_cnt<2)
        {
            auto ret=read(fd,buf,7);
            if(ret>0)
            {
                recv_cnt++;
            }
            else {
                break;
            }
        }

    });
    std::string test_data="check";
    while(send_cnt-->0&&isrunning)
    {
        auto ret=write(fd,test_data.c_str(),test_data.size());
        AIMY_INFO("write %s ret=%d",test_data.c_str(),ret);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(recv_cnt>=2)break;
    }
    ::close(fd);
    if(recv_cnt>=2)
    {
        handle_result(0);
    }
    else {
        handle_result(1,"just recv "+std::to_string(recv_cnt)+" packets,need 2!");
    }
    recv_thread.detach();
}
