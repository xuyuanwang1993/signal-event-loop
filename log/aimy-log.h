#pragma once
#ifndef C_LOG_H
#define C_LOG_H
/*
 * compile with -std=c++11
 * complie with -DBACKTRACE and link with -rdynamic to open backtrace support
 * you can call print_backtrace to log the backtrace info
 */
#include <cstdio>
#include<cstdarg>
#include<string>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<atomic>
#include<functional>
#include<thread>
#include<future>
#if defined(__linux) || defined(__linux__)
#elif defined(WIN32) || defined(_WIN32)
#endif
#if defined(__linux) || defined(__linux__)
#define DIR_DIVISION "/"
#elif defined(WIN32) || defined(_WIN32)
#define DIR_DIVISION "\\"
#pragma warning (disable: 4996)
#pragma
#endif
extern void signal_exit_func(int signal_num);
namespace aimy {
enum LOG_LEVEL{
    LOG_DEBUG=0,
    LOG_INFO,
    LOG_WARNNING,
    LOG_ERROR,
    LOG_FATALERROR,
    LOG_BREAK_POINT,
    LOG_BACKTRACE,
};
using MAX_LOG_CACHE_CALLBACK=std::function<void (const std::string&)>;
struct AimyLoggerBuffer{
    const LOG_LEVEL level;
    const std::string file;
    const std::string func;
    const int line;
    std::shared_ptr<char> log_info;
    uint32_t size;
    uint32_t fill_size;
    AimyLoggerBuffer(uint32_t reserver_size,LOG_LEVEL _level,const char * _file,const char * _func,int line);
    ~AimyLoggerBuffer();
    void append(uint32_t max_len,const char * fmt, ... );
    void print();
};

class AimyLogger{
#if defined(__linux) || defined(__linux__)
    const char LINE_END[2]={'\n','\0'};
#elif defined(WIN32) || defined(_WIN32)
    const char LINE_END[3]={'\r','\n','\0'};
#endif
    const uint64_t MAX_LOG_MESSAGE_SIZE=4096-1;//4k
    const LOG_LEVEL MIN_LOG_LEVEL=LOG_DEBUG;
    const LOG_LEVEL MAX_LOG_LEVEL=LOG_FATALERROR;
    const int64_t MAX_LOG_FILE_SIZE=2LL*1024LL*1024LL*1024LL;//2G
    const int64_t MIN_LOG_FILE_SIZE=32*1024;//32k
    const int WAIT_TIME=1000;//ms
    const int MAX_WRITE_ERROR_TRY=10;
    const unsigned long MAX_LOG_QUEUE_SIZE=100000;
public:
    AimyLogger &operator=(const AimyLogger &) = delete;
    AimyLogger(const AimyLogger &) = delete;
    /*单例*/
    static AimyLogger & Instance()
    {
        if(m_custom_instance)return *m_custom_instance;
        static AimyLogger logger;return logger;
    }
    /**
     * @brief register_exit_signal_func 注册进程信号捕获退出回调函数,此调用非线程安全
     * @param exit_func 退出回调函数
     */
    static void register_exit_signal_func(std::function<void()>exit_func);
    /*log接口*/
    void log(int level,const char *file,const char *func,int line,const char *fmt,...);
    /*log接口*/
    void log(const AimyLoggerBuffer & buffer);
    /*设置log目录，当clear_flag 为true时 会清空原有log,目录无权限时会返回false*/
    bool set_log_path(const std::string &path,const std::string &proname);
    /*设置最小打印等级*/
    void set_minimum_log_level(int level);
    /*设置是否在新建目录时清除log或log文件超出大小时删除原有log*/
    void set_clear_flag(bool clear);
    /*设置单个文件最大大小*/
    void set_log_file_size(long size);
    /**
         * @brief set_max_log_file_cnts 设置日志文件最大数量
         */
    void set_max_log_file_cnts(int cnt);
    /*设置的回调函数*/
    void set_log_callback(const MAX_LOG_CACHE_CALLBACK &callback);
    /*标准输出开关*/
    void set_log_to_std(bool flag);
    /*获取当前本地时间*/
    static std::string get_time_str();
    void register_handle();
    void unregister_handle();
    /**
     * @brief processReset 跨进程后重置状态
     */
    void processReset();
    bool get_register_status()const{
        return m_registered;
    }
    //获取堆栈信息
    static void print_backtrace(int dumpSize=20);
    static std::string abi_convert(const std::string &input);
    static void register_custom_instance(AimyLogger *instance)
    {
        m_custom_instance=instance;
    }
    static AimyLogger*create(){return new AimyLogger;}
    static size_t getThreadId();
    static bool setThreadName(const std::string &name);
    static std::string getThreadName();
    static std::string formatHexToString(const void *input,uint32_t len,bool withSpace=true);
private:
    AimyLogger();
    ~AimyLogger();
    /*判断文件是否打开*/
    bool open_file();
    /*追加到文件中*/
    inline bool append_to_file(const std::string & log_message);
    /*重置文件信息*/
    void reset_file();
    /**
         * @brief rename_file log写入完成，对其进行重新命名
         */
    void rename_file();
    /*log处理线程*/
    void run();
    std::unique_ptr<std::thread>m_thread;
    std::atomic<bool> m_registered;
    std::condition_variable m_conn;
    /*停止标识*/
    std::atomic<bool> m_stop;
    /*文件写入指针*/
    FILE *m_fp;
    /*log目录*/
    std::string m_save_path;
    /*程序名*/
    std::string m_pro_name;
    /*目录是否可写*/
    bool m_env_set;
    /*配置读写锁*/
    std::mutex m_mutex;
    /*LOG等级*/
    std::atomic<int> m_level;
    /*log清理标识*/
    bool m_clear_flag;
    /*log文件最大大小*/
    int64_t m_max_file_size;
    /*log缓存*/
    std::queue<std::shared_ptr<std::string>>  m_log_buf;
    /*回调函数*/
    MAX_LOG_CACHE_CALLBACK m_log_callback;
    /*写入错误累计*/
    int m_write_error_cnt;
    /*标准输出开关*/
    std::atomic_bool m_log_to_std;
    /**
     * @brief m_log_file_cache_max_cnts 日志文件最大数量
     */
    uint32_t m_log_file_cache_max_cnts;
    //信号退出函数
    static std::function<void()>m_exit_func;
    //信号捕获次数，用于强制退出
    static std::atomic_bool m_signal_catch_flag;
    //将exit_func声明为友元函数
    friend void ::signal_exit_func(int signal_num);
    //
    static AimyLogger *m_custom_instance;
};
}
//define log macro
#undef AIMY_LOG
#undef AIMY_DEBUG
#undef AIMY_INFO
#undef AIMY_WARNNING
#undef AIMY_ERROR
#undef AIMY_FATALERROR
#undef AIMY_BACKTRACE
#ifndef AIMY_NO_LOG
#define AIMY_LOG(level,fmt,...) aimy::AimyLogger::Instance().log(level,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_DEBUG(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_DEBUG,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_INFO(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_INFO,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_WARNNING(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_WARNNING,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_ERROR(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_ERROR,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_FATALERROR(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_FATALERROR,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_MARK(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_BREAK_POINT,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_BACKTRACE(fmt,...) aimy::AimyLogger::Instance().log(aimy::LOG_BACKTRACE,__FILE__, __FUNCTION__,__LINE__, fmt, ##__VA_ARGS__)
#define AIMY_LOG_BUFFER(a,level,size)  aimy::AimyLoggerBuffer a(size,level,__FILE__,__FUNCTION__,__LINE__)
#else
#define AIMY_LOG(level,fmt,...)
#define AIMY_DEBUG(fmt,...)
#define AIMY_INFO(fmt,...)
#define AIMY_WARNNING(fmt,...)
#define AIMY_ERROR(fmt,...)
#define AIMY_FATALERROR(fmt,...)
#define AIMY_MARK(fmt,...)
#define AIMY_BACKTRACE(fmt,...)
#endif

#endif // C_LOG_H
