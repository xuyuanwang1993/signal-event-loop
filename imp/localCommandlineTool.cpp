#include "localCommandlineTool.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/un.h>
#include<sys/time.h>
using namespace aimy::Local;
static const uint32_t max_path_size=108;
static  std::string tag="commandline";
static std::pair<std::shared_ptr<uint8_t>, uint32_t> base64Encode(const void *input_buf, uint32_t buf_size)
{
    static const char base64Char[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::shared_ptr<uint8_t>ret_buf=nullptr;
    uint32_t ret_len=0;
    if(buf_size==0||!input_buf) return std::make_pair(ret_buf,ret_len);
    auto buf=reinterpret_cast<const uint8_t*>(input_buf);
    unsigned const numOrig24BitValues = buf_size/3;
    bool havePadding = buf_size > numOrig24BitValues*3;//判断是否有余数
    bool havePadding2 = buf_size == numOrig24BitValues*3 + 2;//判断余数是否等于2
    unsigned const numResultBytes = 4*(numOrig24BitValues + havePadding);//计算最终结果的size
    ret_len=numResultBytes;
    ret_buf.reset(new uint8_t[ret_len+1],std::default_delete<uint8_t[]>());
    memset(ret_buf.get(),0,ret_len+1);
    unsigned i;
    for (i = 0; i < numOrig24BitValues; ++i) {
        ret_buf.get()[4*i+0] = base64Char[(buf[3*i]>>2)&0x3F];
        ret_buf.get()[4*i+1] = base64Char[(((buf[3*i]&0x3)<<4) | (buf[3*i+1]>>4))&0x3F];
        ret_buf.get()[4*i+2] = base64Char[((buf[3*i+1]<<2) | (buf[3*i+2]>>6))&0x3F];
        ret_buf.get()[4*i+3] = base64Char[buf[3*i+2]&0x3F];
    }
    //处理不足3位的情况
    //余1位需在后面补齐2个'='
    //余2位需补齐一个'='
    if (havePadding) {
        ret_buf.get()[4*i+0] = base64Char[(buf[3*i]>>2)&0x3F];
        if (havePadding2) {
            ret_buf.get()[4*i+1] = base64Char[(((buf[3*i]&0x3)<<4) | (buf[3*i+1]>>4))&0x3F];
            ret_buf.get()[4*i+2] = base64Char[(buf[3*i+1]<<2)&0x3F];
        } else {
            ret_buf.get()[4*i+1] = base64Char[((buf[3*i]&0x3)<<4)&0x3F];
            ret_buf.get()[4*i+2] = '=';
        }
        ret_buf.get()[4*i+3] = '=';
    }
    return std::make_pair(ret_buf,ret_len);
}

static std::pair<std::shared_ptr<uint8_t>,uint32_t> base64Decode(const void *input_buf, uint32_t buf_size)
{
    char base64DecodeTable[256];
    int i;//初始化映射表
    for (i = 0; i < 256; ++i) base64DecodeTable[i] = (char)0x80;
    for (i = 'A'; i <= 'Z'; ++i) base64DecodeTable[i] = 0 + (i - 'A');
    for (i = 'a'; i <= 'z'; ++i) base64DecodeTable[i] = 26 + (i - 'a');
    for (i = '0'; i <= '9'; ++i) base64DecodeTable[i] = 52 + (i - '0');
    base64DecodeTable[(unsigned char)'+'] = 62;
    base64DecodeTable[(unsigned char)'/'] = 63;
    base64DecodeTable[(unsigned char)'='] = 0;
    if(buf_size==0||!input_buf) return make_pair(std::shared_ptr<uint8_t>(),0);
    int k=0;
    int paddingCount = 0;
    uint32_t ret_size;
    int const jMax = buf_size - 3;
    ret_size=3*buf_size/4;
    std::shared_ptr<uint8_t>ret_buf(new uint8_t[ret_size],std::default_delete<uint8_t[]>());
    auto buf=reinterpret_cast<const char*>(input_buf);
    for(int j=0;j<jMax;j+=4)
    {
        char inTmp[4], outTmp[4];
        for (int i = 0; i < 4; ++i) {
            inTmp[i] = buf[i+j];
            if (inTmp[i] == '=') ++paddingCount;
            outTmp[i] = base64DecodeTable[(unsigned char)inTmp[i]];
            if ((outTmp[i]&0x80) != 0) outTmp[i] = 0; // this happens only if there was an invalid character; pretend that it was 'A'
        }
        ret_buf.get()[k++]=(outTmp[0]<<2) | (outTmp[1]>>4);
        ret_buf.get()[k++] = (outTmp[1]<<4) | (outTmp[2]>>2);
        ret_buf.get()[k++] = (outTmp[2]<<6) | outTmp[3];
    }
    return make_pair(ret_buf,ret_size-paddingCount);
}
localCommandLineTestTool::localCommandLineTestTool():m_threadRunning(false)
{

}

void localCommandLineTestTool::initServer(const std::string &serverHost)
{
    m_host=serverHost+".service";
    m_isserver=true;
    do{
        sock.reset(new unix_dgram_socket(m_host));
        m_socket_fd=sock->build();
        if(m_socket_fd<=0)
        {
            fprintf(stderr,"init unix socket %s failed[%s]\n",m_host.c_str(),strerror(errno));
            return;
        }
        defaultServerInit();
        AIMY_WARNNING("init server success ->%s",m_host.c_str());
        return;
    }while(0);
}
void localCommandLineTestTool::initClient(bool quickExit, const std::string &serverHost, uint32_t maxRecvCnt, uint32_t maxWaitMsec, bool keepAlive)
{
    m_quickExit=quickExit;
    m_host=serverHost+".service";
    m_maxRecvCnt=maxRecvCnt;
    m_recvWaitMsec=maxWaitMsec;
    m_keepAlive=keepAlive;
    if(m_recvWaitMsec==0)m_recvWaitMsec=200;
    do{
        std::string path=serverHost+"-"+std::to_string(getpid())+".client";
        sock.reset(new unix_dgram_socket(path));
        m_socket_fd=sock->build();
        if(m_socket_fd<=0)
        {
            fprintf(stderr,"init unix socket %s failed[%s]\n",path.c_str(),strerror(errno));
            return;
        }
        AIMY_DEBUG("init client success ->%s:%hu",m_host.c_str());
        return;
    }while(0);
}
void localCommandLineTestTool::addTestCommand(ExternalParam &data)
{
    std::unique_lock<std::mutex>locker(m_rawDataMutex);
    if(m_clientDataList.size()>=1024)m_clientCv.wait_for(locker,std::chrono::milliseconds(30));
    m_clientDataList.push_back(data);
    m_clientCv.notify_one();
}

void localCommandLineTestTool::handleCommandlineCmd(int argc,char *argv[])
{
    if(argc==1)
    {
        AIMY_DEBUG("%s",getHelpInfo().c_str());
        exit(0);
    }
    argc--;
    argv++;
    // parse host_name  host_port
    std::string host_name=LOCAL_SERVICE_NAME;
    uint32_t recv_cnt=1;
    uint32_t timeout_msec=200;
    bool keep_alive=false;
    bool isServer=false;
    while(argc>0)
    {
        std::string opt=*argv;
        argv++;
        argc--;
        if(opt=="--host"){
            if(argc>0)
            {
                host_name=*argv;
                argv++;
                argc--;
            }
        }
        else if (opt=="--cnt") {
            if(argc>0)
            {
                recv_cnt=std::stoul(*argv)&0xffffffff;
                argv++;
                argc--;
            }
        }
        else if (opt=="--timeout") {
            if(argc>0)
            {
                timeout_msec=std::stoul(*argv)&0xffffffff;
                argv++;
                argc--;
            }
        }
        else if (opt=="--keepalive") {
            if(argc>0)
            {
                keep_alive=std::stoul(*argv)!=0;
                argv++;
                argc--;
            }
        }
        else{
            if(opt=="server")isServer=true;
            else{

                ExternalParam data;
                ExternalParamList dataList;
                dataList.push_back(base64Encode(opt.c_str(),opt.size()));
                while (argc>0) {
                    dataList.push_back(base64Encode(argv[0],strlen(argv[0])));
                    argc--;
                    argv++;
                }
                uint32_t data_len=0;
                for(auto i:dataList)
                {
                    data_len+=i.second+1;
                }
                data=createExternalParam(data_len);
                uint32_t offset=0;
                for(auto i:dataList)
                {
                    if(offset>0)
                    {
                        data.first.get()[offset++]=PARAM_SPLIT_CHAR;
                    }
                    memcpy(data.first.get()+offset,i.first.get(),i.second);
                    offset+=i.second;
                }
                data.second=offset;
                addTestCommand(data);
            }
            break;
        }
    }
    if(isServer)initServer(host_name);
    else{
        initClient(keep_alive?false:true,host_name,recv_cnt,timeout_msec,keep_alive);
    }
}
bool localCommandLineTestTool::insertCallback(const std::string&name, const std::string &help_info, const ExternalFunction&func, uint32_t paramCount)
{
    bool ret=false;
    do{
        auto iter=m_callbackMaps.find(name);
        if(iter!=std::end(m_callbackMaps))break;
        testCallParam param;
        param.callInfo=help_info;
        param.callFunc=func;
        param.paramCount=paramCount;
        m_callbackMaps.insert(std::make_pair(name,param));
        ret=true;
    }while(0);
    return ret;
    ;
}
localCommandLineTestTool::~localCommandLineTestTool()
{
    stop();
    waitDone();
    if(m_socket_fd>0)::close(m_socket_fd);
    m_clientDataList.clear();
}




std::string localCommandLineTestTool::invokeCallback(const std::string&func_name, const ExternalParamList &params)
{
    std::string ret=func_name+" ";
    auto iter=m_callbackMaps.find(func_name.c_str());
    if(iter!=std::end(m_callbackMaps)){
        auto paramCount=iter->second.paramCount;
        if(params.size()<paramCount)
        {
            ret+=" too few param ";
            return ret;
        }
        return ret+iter->second.callFunc(params);
    }
    else{
        return ret+"undefined";
    }
}
std::string localCommandLineTestTool::getHelpInfo(bool print_func_info,const std::string &func_name)
{
    static const std::string base_help_info="\r\n"\
                                            "*******************************************************\r\n"\
                                            "                     命令行测试工具1.0                   \r\n"\
                                            "*******************************************************\r\n"\
                                            "Usage:\r\n"\
                                            "\tproc_name [--host <host_name>][--cnt <client max recv cnts>]\r\n"
                                            "[--timeout <client recv timeout msec>][--keepalive <0|1 >]<command>\r\n"\
                                            "command:\r\n"\
                                            "\thelp|--help|-h [0|1 [func_name]]\t获取帮助信息\r\n"\
                                            "\trun <func_name> [paramlist]\t运行相应的函数\r\n"\
                                            "\tserver \t以测试服务器模式运行\r\n"\
            ;
    std::string help_info=base_help_info;
    if(print_func_info){
        help_info+="\r\n";
        help_info+=getInvokeCallbackInfo(func_name);
    }
    return  help_info;
}
std::string localCommandLineTestTool::getInvokeCallbackInfo(const std::string &func_name)
{
    std::string ret;
    do{
        if(!func_name.empty())
        {
            ret="----specified func info-----\r\n";
            ret=func_name+" : ";
            auto iter=m_callbackMaps.find(func_name.c_str());
            if(iter==std::end(m_callbackMaps))break;
            ret+=iter->second.callInfo;
        }else{
            ret="-----all callable func info-----";
            for(auto i:m_callbackMaps)
            {
                if(!ret.empty())ret+="\r\n";
                ret+=i.first+" : "+i.second.callInfo;
            }
        }
    }while(0);
    return ret;
}

ExternalParam localCommandLineTestTool::createExternalParam(uint32_t len,const void *copyBuf)
{
    ExternalParam ret;
    do{
        if(len==0)break;
        ret.first.reset(new uint8_t[len+1],std::default_delete<uint8_t[]>());
        memset(ret.first.get(),0,len+1);
        if(copyBuf)
        {
            memcpy(ret.first.get(),copyBuf,len);
        }
        ret.second=len;
    }while(0);
    return  ret;
}

ExternalParamList localCommandLineTestTool::parse(const ExternalParam&param)
{
    ExternalParamList ret;
    uint32_t start_pos=0;
    uint32_t offset=0;
    uint32_t total_len=param.second;
    auto data_ptr=param.first.get();
    if(!data_ptr||total_len==0)return ret;
    auto parse_func=[&ret](const uint8_t *data_ptr,uint32_t start_pos,uint32_t offset)
    {
        if(offset<=start_pos)return ;
        ret.push_back(base64Decode(data_ptr+start_pos,offset-start_pos));
    };
    //filter begin 0xff
    while(start_pos<total_len&&data_ptr[start_pos]==PARAM_SPLIT_CHAR)++start_pos;
    offset=start_pos;
    while(offset<total_len)
    {
        if(data_ptr[offset]!=PARAM_SPLIT_CHAR){
            ++offset;
            continue;
        }
        parse_func(data_ptr,start_pos,offset);
        start_pos=offset+1;
        while(start_pos<total_len&&data_ptr[start_pos]==PARAM_SPLIT_CHAR)++start_pos;
        offset=start_pos;
    }
    parse_func(data_ptr,start_pos,total_len);
    return ret;
}

std::string localCommandLineTestTool::paramToString(const ExternalParam&param)
{
    if(param.second==0||!param.first.get())return std::string();
    return std::string(reinterpret_cast<const char *>(param.first.get()),param.second);
}

std::string localCommandLineTestTool::handleRequest(ExternalParam&param)
{
    auto param_list=parse(param);
    if(param_list.empty())return "invalid input!";
    auto iter=param_list.begin();
    auto mode_name=std::string(reinterpret_cast<const char *>(iter->first.get()),iter->second);
    ++iter;
    if(mode_name=="help"||mode_name=="--help"||mode_name=="-h")
    {
        bool print_func_info=false;
        if(iter!=param_list.end())
        {
            print_func_info=std::stoi(paramToString(*iter));
            ++iter;
        }
        std::string func_name;
        if(iter!=param_list.end())
        {
            func_name=paramToString(*iter);
        }
        return getHelpInfo(print_func_info,func_name).c_str();
    }
    else if(mode_name=="run")
    {
        if(iter==param_list.end()){
            return std::string("request parse failed, error:[no specified func name]");
        }
        std::string parse_result;
        for(auto i:param_list)
        {
            if(!parse_result.empty())parse_result+=" ";
            parse_result+=paramToString(i);
        }
        AIMY_INFO("exec:[%s]",parse_result.c_str());
        std::string func_name=paramToString(*iter);
        ++iter;
        ExternalParamList funcParamList;
        for(;iter!=param_list.end();++iter)
        {
            funcParamList.push_back(*iter);
        }
        return invokeCallback(func_name,funcParamList).c_str();
    }
    else if (mode_name=="keepalive") {
        return "";
    }
    else{
        return std::string(mode_name)+" not supported!";
    }
}

void localCommandLineTestTool::start()
{
    if(m_threadRunning.load())return;
    stop();
    waitDone();
    m_threadRunning.exchange(true);
    std::lock_guard<std::mutex>locker(m_thread_mutex);
    m_workThread=new std::thread([this](){
        AimyLogger::setThreadName("l_workThread");
        loop();
    });

}

void localCommandLineTestTool::stop()
{
    m_threadRunning.exchange(false);
}

void localCommandLineTestTool::waitDone()
{
    std::lock_guard<std::mutex>locker(m_thread_mutex);
    if(m_workThread&&m_workThread->joinable())
    {
        m_workThread->join();
        delete m_workThread;
        m_workThread=nullptr;
    }
}

void localCommandLineTestTool::multicastMessage(const std::string &message)
{
    std::lock_guard<std::mutex>locker(m_multicastMutex);
    auto iter=m_addrDict.begin();
    timeval now;
    gettimeofday(&now,nullptr);
    while(iter!=m_addrDict.end())
    {
        auto time_old=iter->second;
        auto time_diff=(now.tv_sec-time_old.tv_sec)*1000+(now.tv_usec-time_old.tv_usec)/1000;
        if(time_diff>10000)//10s
        {
            iter=m_addrDict.erase(iter);
            continue;
        }
        AIMY_DEBUG("send to %s %s",iter->first.c_str(),message.c_str());
        auto ret=sock->send(message.c_str(),message.length(),iter->first);
        if(ret>0)
        {
            ++iter;
        }
        else {
            iter=m_addrDict.erase(iter);
        }
    }
}

void localCommandLineTestTool::defaultServerInit()
{
    insertCallback("echo","str print",[](const ExternalParamList&paramlist){
        std::string ret="{";
        for(auto i:paramlist)
        {
            ret+=std::string("[")+paramToString(i)+"]";
        }
        ret+="}";
        return ret;
    },1);
}

void localCommandLineTestTool::processClientResponse()
{
    uint32_t buf_size=32*1024;
    char buf[buf_size];
    //wait answer
    while(1)
    {
        memset(buf,0,buf_size);
        auto recv_len=sock->recv(buf,buf_size,m_recvWaitMsec);
        if(recv_len>0)
        {
            uint32_t slice_size=4000;
            uint32_t debug_size=0;
            for(uint32_t offset=0;offset<recv_len;offset+=debug_size)
            {
                debug_size=recv_len-offset;
                if(debug_size>slice_size)debug_size=slice_size;
                AIMY_DEBUG("%s",std::string(buf+offset,debug_size).c_str());
            }
        }
        else {
            break;
        }
    }

}

void localCommandLineTestTool::loop()
{
    if(m_socket_fd<=0)
    {
        AIMY_DEBUG("init error!");
        return;
    }
    while(m_threadRunning)
    {
        if(m_isserver)
        {
            if(!serverTask())break;
        }
        else {
            if(!clientTask()){
                uint32_t recv_cnt=m_maxRecvCnt;
                while((m_maxRecvCnt==0)||recv_cnt>0)
                {
                    recv_cnt--;
                    processClientResponse();
                }
                break;
            }
        }
    }
    m_threadRunning.exchange(false);
}


 bool localCommandLineTestTool::clientTask()
 {
     std::unique_lock<std::mutex>locker(m_rawDataMutex);
     if(m_clientDataList.empty())
     {
         if(m_quickExit)return false;
         m_clientCv.notify_one();
         m_clientCv.wait_for(locker,std::chrono::seconds(5));
         if(m_clientDataList.empty()&&m_keepAlive)
         {
             std::list<std::string>input;
             input.push_back("keepalive");
             locker.unlock();
             appendClientData(input);
             locker.lock();
         }
     }
     if(m_clientDataList.empty())return false;
     auto data=m_clientDataList.front();
     m_clientDataList.pop_front();
     locker.unlock();
     const static uint32_t buf_size=32*1024;
     std::shared_ptr<char>buf(new char[buf_size],std::default_delete<char[]>());
     memset(buf.get(),0,buf_size);
     auto path=sock->get_local_path();
     memcpy(buf.get(),path.c_str(),path.size());
     memcpy(buf.get()+max_path_size,data.first.get(),data.second);
     auto send_len=sock->send(buf.get(),max_path_size+data.second,m_host);
     if(static_cast<decltype (max_path_size+data.second)>(send_len)!=(max_path_size+data.second))
     {
         AIMY_DEBUG("send error[%s]",strerror(errno));
         return false;
     }
     processClientResponse();
     return true;
 }

void localCommandLineTestTool::appendClientData(std::list<std::string>inputList)
{
    if(m_isserver)return;
    ExternalParam data;
    ExternalParamList dataList;
    for(auto &i:inputList)
    {
       dataList.push_back(base64Encode(i.c_str(),i.size()));
    }
    uint32_t data_len=0;
    for(auto i:dataList)
    {
        data_len+=i.second+1;
    }
    data=createExternalParam(data_len);
    uint32_t offset=0;
    for(auto i:dataList)
    {
        if(offset>0)
        {
            data.first.get()[offset++]=PARAM_SPLIT_CHAR;
        }
        memcpy(data.first.get()+offset,i.first.get(),i.second);
        offset+=i.second;
    }
    data.second=offset;
    addTestCommand(data);
}


bool localCommandLineTestTool::serverTask()
{
    uint32_t buf_len=32*1024;
    char recv_buf[buf_len];
    auto recv_len=sock->recv(recv_buf,buf_len,5000);
    if(recv_len==0){
        AIMY_DEBUG("recv error[%s]",strerror(errno));
        return false;
    }
    else if (recv_len<0) {
        return true;
    }
    else if(recv_len>max_path_size){
        std::string source_path(recv_buf,max_path_size);
        ExternalParam param=createExternalParam(recv_len-max_path_size,recv_buf+max_path_size);
        {
            std::lock_guard<std::mutex>locker(m_multicastMutex);
            timeval now;
            gettimeofday(&now,nullptr);
            m_addrDict[source_path]=now;
        }
        auto ret=handleRequest(param);
        if(!ret.empty())
        {
            multicastMessage(ret);
        }
    }
    return true;
}

