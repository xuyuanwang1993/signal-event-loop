#ifndef ICACHE_BUFFER_STREAM_H
#define ICACHE_BUFFER_STREAM_H
#include "base/icache_buffer.h"
namespace aimy {

class IcacheBufferStream:public IcacheBufferBase{
public:
    IcacheBufferStream(std::shared_ptr<ProtocalBase> _protocal,SOCKET _fd);
    ~IcacheBufferStream () override;
    ssize_t readFromFd () override;
    ssize_t readFromBuf(const void *data,uint32_t buf_len) override;
    ssize_t sendCacheByFd() override;
};
}
#endif // ICACHE_BUFFER_STREAM_H
