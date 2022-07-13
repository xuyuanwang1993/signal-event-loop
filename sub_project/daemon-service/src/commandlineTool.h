#ifndef COMMANDLINETOOL_H
#define COMMANDLINETOOL_H
#include<functional>
#include<map>
#include<list>
#include<string>
#include <unordered_map>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<atomic>
#include "unix-socket-helper.h"
#include "network-log.h"
namespace aimy {
using ExternalParam=std::pair<std::shared_ptr<uint8_t>,uint32_t>;
using ExternalParamList=std::list<ExternalParam>;
using ExternalFunction=std::function<std::string (const ExternalParamList&)>;
class commandLineTestTool{
    struct testCallParam{
        ExternalFunction callFunc;
        uint32_t paramCount=0;
        std::string callInfo;
    };

    constexpr static uint8_t PARAM_SPLIT_CHAR=0xff;
public:
    explicit commandLineTestTool();
    /**
     * @brief init_server 启动udp服务器
     * @param serverHost
     * @param serverPort
     */
    void initServer(const std::string &serverHost="127.0.0.1",uint16_t serverPort=8888);
    /**
     * @brief init_client 建立与服务器的连接
     * @param serverHost
     * @param serverPort
     */
    void initClient(bool quickExit,const std::string &serverHost="127.0.0.1",uint16_t serverPort=8888);
    /**
     * @brief addTestCommand 插入测试命令数据
     * @param data 数据列表

     */
    void addTestCommand(ExternalParam&data);
    /**
     * @brief handleCommandlineCmd 解析命令行的输入数据
     * @param argc
     * @param argv
     */
    void handleCommandlineCmd(int argc,char *argv[]);
    /**
     * @brief insertCallback 增加测试指令
     * @param name 测试指令名
     * @param help_info 测试指令描述信息，需表明如何传递参数
     * @param func 测试命令函数体
     * @param paramList 参数解析规则
     * @return
     * 测试函数的返回值将会被返回到调用它的客户端
     */
    bool insertCallback(const std::string&name, const std::string &help_info, const ExternalFunction&func, uint32_t paramCount);
    void appendClientData(std::list<std::string> inputList);
    std::string getHelpInfo(bool print_func_info=false,const std::string &func_name=std::string());
    virtual ~commandLineTestTool();
    void start();
    void stop();
    void waitDone();
protected:
    virtual void defaultServerInit();
private :
    void loop();
    bool clientTask();
    bool clientTask2();
    bool serverTask();
    bool serverTask2();
private:
    std::string invokeCallback(const std::string&func_name,const ExternalParamList&params);
    std::string getInvokeCallbackInfo(const std::string &func_name=std::string());
    std::string handleRequest(ExternalParam&param);
    static ExternalParam createExternalParam(uint32_t len,const void *copyBuf=nullptr);
    static ExternalParamList parse(const ExternalParam&param);
    static std::string paramToString(const ExternalParam&param);
private:
    bool m_isserver=false;
    bool m_quickExit=true;
    std::map<std::string,testCallParam>m_callbackMaps;
    std::list<ExternalParam>m_clientDataList;
    std::mutex m_rawDataMutex;
    std::condition_variable m_clientCv;
    std::string m_host;
    uint16_t m_port = 0;
    int m_socket_fd=-1;
    std::atomic<bool>m_threadRunning;
    std::mutex m_thread_mutex;
    std::thread *m_workThread=nullptr;
    std::shared_ptr<unix_dgram_socket>sock;
};
}
#endif // COMMANDLINETOOL_H
