#ifndef PROTOCALBASE_H
#define PROTOCALBASE_H
#include <string>
#include<cstdint>
#include<memory>
#include<list>
namespace aimy {

class ProtocalBase{
public:
    static constexpr uint32_t PROTOCAL_NO_LIMIT_SIZE=0;
public:
    ProtocalBase(uint32_t _max_cache_size,bool _is_streaming,uint32_t _max_frame_size,uint32_t _slice_size);
    virtual ~ProtocalBase();

    virtual uint32_t getMaxCacheSize()const {return max_cache_size;}
    virtual uint32_t isStreaming() const { return is_streaming; }
    virtual uint32_t getMaxFrameSize()const { return max_frame_size; };
    virtual uint32_t getSliceSize()const { return slice_size; } ;
    virtual std::pair<std::shared_ptr<uint8_t>,uint32_t> encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list);
    /**
     * @brief decodeFrame
     * @param data
     * @param len
     * @return  0 incomplete frame 1->max_frame_size available frame > max_fram_size invalid data,invalid data length= ret.second-getMaxFrameSize
     */
    virtual std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> decodeFrame(const void *data,uint32_t len);
protected:
    const uint32_t max_cache_size;
    const bool is_streaming;
    const uint32_t max_frame_size;
    const uint32_t slice_size;
};
}
#endif // PROTOCALBASE_H
