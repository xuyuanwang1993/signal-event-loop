#ifndef ICACHE_BUFFER_H
#define ICACHE_BUFFER_H
#include "core/core-include.h"
#include "protocal-base.h"
#include <cstdint>
namespace aimy {
class IcacheBufferBase {
public:
    IcacheBufferBase( std::shared_ptr<ProtocalBase> _protocal, SOCKET _fd, uint32_t _max_cache_size);
    virtual ~IcacheBufferBase();
    //read
    /**
     * @brief readFromFd
     * @return -1 wait next call 0 close >0 success
     */
    virtual ssize_t readFromFd();
    virtual ssize_t readFromBuf(const void *data,uint32_t buf_len);
    std::pair<std::shared_ptr<uint8_t>,uint32_t> popFrame();
    //write
    /**
     * @brief sendCacheByFd
     * @return -1 wait next call 0 close >0 success
     */
    virtual ssize_t sendCacheByFd();
    virtual ssize_t appendRawData(const void *data,uint32_t len);
    virtual ssize_t appendFrame(const void *data,uint32_t len);
    //
    uint32_t frame_count () const { return frame_cache_map.size(); };
protected:
    void resize(uint32_t new_size);
    bool appendCache(const void *data,uint32_t len);
    std::pair<uint32_t,uint32_t> parseCache();
    void updateFrameCache(uint32_t frame_offset, uint32_t frame_len);
protected:
    std::shared_ptr<ProtocalBase> protocal;
    SOCKET fd;
    //cache
    const uint32_t max_cache_size;
    std::shared_ptr<uint8_t> cache_buf;
    uint32_t cache_size;
    uint32_t write_offset;
    uint32_t decode_offset;
    std::list<std::pair<uint32_t/*frame begin offset to frame_offset*/,uint32_t/*frame length*/>>frame_cache_map;
    //
    std::shared_ptr<uint8_t> read_buffer;
    uint32_t read_buffer_size;
};
}
#endif // ICACHE_BUFFER_H
