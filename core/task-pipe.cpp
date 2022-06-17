#include "task-pipe.h"
using namespace aimy;
Pipe::Pipe(){

}
bool Pipe::open()
{
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
    //鉴于socket是一个双向管道，故此例从写管道写入到自身的读管道
    //windows上使用回环写入
    if(m_fd== INVALID_SOCKET){
        m_fd=NETWORK_UTIL::build_socket(NETWORK_UTIL::UDP);
        NETWORK_UTIL::bind(m_fd);
        NETWORK_UTIL::connect(m_fd, "127.0.0.1", NETWORK_UTIL::get_local_port(m_fd));
        NETWORK_UTIL::make_noblocking(m_fd);
    }
    return m_fd!= INVALID_SOCKET;
#elif defined(__linux) || defined(__linux__)
    if (pipe2(m_fd, O_NONBLOCK | O_CLOEXEC) < 0)
    {
        return false;
    }
    return true;
#endif
}
void Pipe::close()
{
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
    if(m_fd!= INVALID_SOCKET)NETWORK_UTIL::close_socket(m_fd);
    m_fd= INVALID_SOCKET;
#elif defined(__linux) || defined(__linux__)
    if(m_fd[0]!= INVALID_SOCKET)NETWORK_UTIL::close_socket(m_fd[0]);
    if(m_fd[1]!= INVALID_SOCKET)NETWORK_UTIL::close_socket(m_fd[1]);
    m_fd[0]=INVALID_SOCKET;
    m_fd[1]=INVALID_SOCKET;
#endif
}
int64_t Pipe::read(void *buf,uint32_t max_len)
{
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
    if(m_fd== INVALID_SOCKET)return  -1;
    return recv(m_fd, buf, max_len, 0);
#elif defined(__linux) || defined(__linux__)
    if(m_fd[0]==INVALID_SOCKET) return -1;
    return ::read(m_fd[0], buf, max_len);
#endif
}
int64_t Pipe::write(const void *buf, uint32_t buf_len)
{
#if defined(WIN32) || defined(_WIN32)||defined (WINDOWS_TEST)
    if(m_fd== INVALID_SOCKET)return  -1;
    return ::send(m_fd, buf, buf_len, 0);
#elif defined(__linux) || defined(__linux__)
    if(m_fd[1]==INVALID_SOCKET) return -1;
    return ::write(m_fd[1], buf, buf_len);
#endif
}
Pipe::~Pipe()
{
    close();
}
