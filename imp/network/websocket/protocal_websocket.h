#ifndef PROTOCAL_WEBSOCKET_H
#define PROTOCAL_WEBSOCKET_H
#include "imp/network/base/protocal-base.h"
#include <map>
#include<vector>
#include<random>
namespace aimy {

// http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |     Extended payload length continued, if payload len == 127  |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               |Masking-key, if MASK set to 1  |
// +-------------------------------+-------------------------------+
// | Masking-key (continued)       |          Payload Data         |
// +-------------------------------- - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+
enum WS_FrameType :uint8_t
{
    WS_CONTINUATION_FRAME=0x0,
    WS_TEXT_FRAME = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_RESERVER_NO_CONTROL_3_FRAME=0x03,
    WS_RESERVER_NO_CONTROL_4_FRAME=0x04,
    WS_RESERVER_NO_CONTROL_5_FRAME=0x05,
    WS_RESERVER_NO_CONTROL_6_FRAME=0x06,
    WS_RESERVER_NO_CONTROL_7_FRAME=0x07,
    WS_CONNECTION_CLOSE_FRAME=0x08,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A,
    WS_RESERVER_CONTROL_B_FRAME=0x0B,
    WS_RESERVER_CONTROL_C_FRAME=0X0C,
    WS_RESERVER_CONTROL_D_FRAME=0X0D,
    WS_RESERVER_CONTROL_E_FRAME=0X0E,
    WS_RESERVER_CONTROL_F_FRAME=0X0F,
};
struct WSFrameHeader{

    //if it is the last frame fragment,set this bit to 1
    uint8_t FIN:1;
    //always be 0
    uint8_t RSV:3;
    //WS_FrameType description the application's frame type
    uint8_t opcode:4;
    //if the frame is encode with mask this bit must be 1
    //usually,mask is must used by websocket_client to send a frame
    uint8_t MASK:1;
    //raw_len <126 payload_len=raw_len
    //raw_len<=65536 payload_len=126
    //raw_len>65536 payload_len=127
    uint8_t payload_len:7;
    //126=<raw_len<=65536 len_for_below_or_equal_65536=raw_len
    //raw_len>65536 len_for_larger_than_65536=raw_len
    union {
        uint16_t len_for_below_or_equal_65536;
        uint64_t len_for_larger_than_65536;
    }extended_payload_len;
    //if payload_len<126 then payload's len=payload_len padding_payload_len=0
    //if payload_len<=65536 then payload's len=len_for_below_or_equal_65536 padding_payload_len=2
    //if payload_len>65536 then payload's len=len_for_larger_than_65536 padding_payload_len=8
    uint8_t padding_payload_len;
    //if MASK bit is set,this filed will be filled
    char Masking_key[4];
    int actual_header_len;
    int64_t actual_payload_len;
    /**
     * @brief generate_random_mask_key generate a random mask key
     */
    static void generate_random_mask_key(char (*buf)[4]){
        std::random_device rd;
        (*buf)[0]=static_cast<char>(rd()%256);
        (*buf)[1]=static_cast<char>(rd()%256);
        (*buf)[2]=static_cast<char>(rd()%256);
        (*buf)[3]=static_cast<char>(rd()%256);
    }
    WSFrameHeader():FIN(0),RSV(0),opcode(0),MASK(0),payload_len(0),padding_payload_len(0),actual_header_len(-1),actual_payload_len(-1){

    }
};

struct StreamWebsocketFrame{
    WSFrameHeader header;
    uint8_t * payload;
    StreamWebsocketFrame():payload(nullptr)
    {}
};

class ProtocalWebsocket :public ProtocalBase{
public:
    ProtocalWebsocket(uint32_t _max_cache_size=10*1024*1024,uint32_t _max_frame_size=10*1024*1024,uint32_t _slice_size=1200);
    ~ProtocalWebsocket() override;
    virtual std::string description()const { return "http"; }

    std::pair<std::shared_ptr<uint8_t>,uint32_t> packetFrame(const void *payload,uint32_t payload_len,bool use_mask,WS_FrameType frame_type,bool is_fin) ;

    std::pair<std::shared_ptr<uint8_t>,uint32_t> encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list) override;
    std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> decodeFrame(const void *data,uint32_t len) override;

    WSFrameHeader decodeFrameHeader(const void *data, uint32_t data_len);
    bool bufferToFrame(void *buf, uint32_t buf_len,StreamWebsocketFrame * frame);
    static void dumpFrame(const std::string &tag,const StreamWebsocketFrame &frame);
protected:
    uint32_t minProtocalSize() override { return 2;}
};
}
#endif // PROTOCAL_WEBSOCKET_H
