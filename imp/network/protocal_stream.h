#ifndef PROTOCAL_STREAM_H
#define PROTOCAL_STREAM_H
#include "base/protocal-base.h"
namespace aimy {

struct StreamFrameHeader{
    uint8_t version:4;//当前固定为1
    uint8_t reserved:4;
    uint8_t check_code;//原始负载校验和  0x100-(header+payload)
    uint16_t header_length;//字节流传输需转成大端字节序
    uint32_t payload_length;//字节流传输需转成大端字节序
    uint64_t timestamp;//字节流传输需转成大端字节序 ms时间戳，早于本机时间5分钟的连接应当被断开
    //    头部+负载数据部分使用时间戳做异或处理
    //    具体算法
    //    uint8_t *map=(uint8_t *)(timestamp);
    //    for(uint32_t offset=0;offset<data_len;++offset)
    //    {
    //        data[offset]^=map[offset%8];
    //    }
    StreamFrameHeader():version(0),reserved(0),check_code(0),header_length(0),payload_length(0),timestamp(0)
    {}
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
    virtual std::string description()const { return "streamV1"; }

    virtual std::pair<std::shared_ptr<uint8_t>,uint32_t> packetFrame(const void *header,uint16_t header_len,const void * payload,uint32_t payload_len) ;

    std::pair<std::shared_ptr<uint8_t>,uint32_t> encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list) override;
    std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> decodeFrame(const void *data,uint32_t len) override;

    StreamFrameHeader decodeFrameHeader(const uint8_t *data);
    bool bufferToFrame(void *buf, uint32_t buf_len,StreamFrame * frame);
protected:
    //static
    static uint8_t caculateCheckcode(const void *header,uint32_t header_len,const void * payload,uint32_t payload_len);
    static void encryptPayload(const void *mask,uint32_t mask_len,void *data,uint32_t data_len);
    //
    uint32_t minProtocalSize() override { return 16;}
};
}
#endif // PROTOCAL_STREAM_H
