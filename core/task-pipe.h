#ifndef TASKPIPE_H
#define TASKPIPE_H
#include "network-util.h"
//#define WINDOWS_TEST
namespace aimy {
class Pipe{
public:
    Pipe();
    /**
     * @brief open 创建通信管道
     * @return
     */
    bool open();
    /**
     * @brief close 关闭通信管道
     */
    void close();
    /**
     * @brief read 从管道中读取数据
     * @param buf 接收buf
     * @param max_len 接收buf长度
     * @return 读到的数据长度
     */
    int64_t read(void *buf,uint32_t max_len);
    /**
     * @brief write 向管道中写入数据
     * @param buf 发送数据缓冲
     * @param buf_len 待发送数据长度
     * @return 实际发送长度
     */
    int64_t write(const void *buf,uint32_t buf_len);
    ~Pipe();
    /**
     * @brief operator () 获取读fd
     * @return
     */
    SOCKET operator ()()const{
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
        return m_fd;
#elif defined(__linux) || defined(__linux__)
        return m_fd[0];
#endif
    }
private:
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
    /**
     * @brief m_fd 通信管道对应的socket
     */
    SOCKET m_fd=INVALID_SOCKET;
#elif defined(__linux) || defined(__linux__)
    SOCKET m_fd[2]={INVALID_SOCKET,INVALID_SOCKET};
#endif
};
}
#endif // TASKPIPE_H
