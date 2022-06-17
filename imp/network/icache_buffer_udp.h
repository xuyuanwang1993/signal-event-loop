#ifndef ICACHE_BUFFER_UDP_H
#define ICACHE_BUFFER_UDP_H
#include "base/icache_buffer.h"
namespace aimy {
/**
 * @brief The IcacheBufferUdp class
 * every packet is a frame, the frame you got is started with sockaddr_in
 * the frame you push into the buffer need starting with its' destination
 */
class IcacheBufferUdp:public IcacheBufferBase{
public:
    IcacheBufferUdp(std::shared_ptr<ProtocalBase> _protocal,SOCKET _fd);
    ~IcacheBufferUdp () override;
    ssize_t readFromFd () override;
    ssize_t sendCacheByFd() override;
};
}
#endif // ICACHE_BUFFER_UDP_H
