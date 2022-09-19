#ifndef PROTOCALHTTP_H
#define PROTOCALHTTP_H
#include "imp/network/base/protocal-base.h"
#include <map>
#include<vector>
namespace aimy {

struct StreamHttpHeader{
    std::vector<std::string> header;
    std::map<std::string,std::string> keys;
    int64_t content_length;
    int64_t header_offset;
    bool parse_error;
    bool process_finished;
    StreamHttpHeader():content_length(-1),header_offset(24),parse_error(false),process_finished(false)
    {}
    void resetHeader()
    {
        keys.clear();
        header.clear();
        parse_error=false;
        header_offset=-1;
       content_length=-1;
    }
};

struct StreamHttpFrame{
    StreamHttpHeader header;
    uint8_t * content;
    StreamHttpFrame():content(nullptr)
    {}
};

class ProtocalHttp :public ProtocalBase{
public:
    ProtocalHttp(uint32_t _max_cache_size=10*1024*1024,uint32_t _max_frame_size=10*1024*1024,uint32_t _slice_size=1200);
    ~ProtocalHttp() override;
    virtual std::string description()const { return "http"; }

    virtual std::pair<std::shared_ptr<uint8_t>,uint32_t> packetFrame(const std::vector<std::string> & header, std::map<std::string,std::string> &keys_map, const void * content, uint32_t content_len) ;

    std::pair<std::shared_ptr<uint8_t>,uint32_t> encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list) override;
    std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> decodeFrame(const void *data,uint32_t len) override;

    void decodeFrameHeader(StreamHttpHeader * header,const char *data,uint32_t data_len);
    bool bufferToFrame(void *buf, uint32_t buf_len,StreamHttpFrame * frame);
protected:
    //static
    static const char * getNextLine(const char *input);
    // maybe a minimum http packet's size will be larger than 24
    uint32_t minProtocalSize() override { return 24;}
protected:
    StreamHttpHeader header_cache;
};
}
#endif // PROTOCALHTTP_H
