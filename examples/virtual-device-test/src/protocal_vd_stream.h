#ifndef PROTOCAL_VD_STREAM_H
#define PROTOCAL_VD_STREAM_H
#include "imp/network/base/protocal-base.h"
namespace aimy {

namespace virtual_device{
struct StreamFrameHeader{
    //
    uint32_t length;//minProtocalSize+json_header_len+payload_len
    uint16_t version;
    uint16_t compress_flag;//0:无压缩; >0:对应具体的压缩方案
    uint32_t stream_offset;//二进制流相对于body的偏移量, ~0:没有二进制流; 0:没有json体;
    StreamFrameHeader():length(0),version(0),compress_flag(0),stream_offset(0)
    {}
    uint32_t getHeaderLen()const
    {
        if(stream_offset!=~0)
        {
            return stream_offset;
        }
        else {
            return length-minSize();
        }
    }
    uint32_t getPayloadLen()const
    {
        if(stream_offset==~0)return 0;
        else {
            return length-minSize()-stream_offset;
        }
    }
    static uint32_t minSize()
    {
        return 12;
    }
};

struct StreamFrame{
    StreamFrameHeader header;
    uint8_t * payload_header;
    uint8_t * payload;
    StreamFrame():payload_header(nullptr),payload(nullptr)
    {}
};

class ProtocalStream :public ProtocalBase{
public:
    ProtocalStream(uint32_t _max_cache_size=10*1024*1024,uint32_t _max_frame_size=10*1024*1024,uint32_t _slice_size=1200);
    ~ProtocalStream() override;
    virtual std::string description()const { return "streamVD"; }

    virtual std::pair<std::shared_ptr<uint8_t>,uint32_t> packetFrame(const void *header,uint16_t header_len,const void * payload,uint32_t payload_len) ;

    std::pair<std::shared_ptr<uint8_t>,uint32_t> encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list) override;
    std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> decodeFrame(const void *data,uint32_t len) override;

    StreamFrameHeader decodeFrameHeader(const uint8_t *data);
    bool bufferToFrame(void *buf, uint32_t buf_len,StreamFrame * frame);
protected:
    virtual uint32_t minProtocalSize(){ return StreamFrameHeader::minSize();}
};
}
}
#endif // PROTOCAL_VD_STREAM_H
