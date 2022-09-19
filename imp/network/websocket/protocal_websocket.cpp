#include "protocal_websocket.h"
#include "core/core-include.h"
#include<cstring>
using namespace aimy;
static const char *const  HEAD_END="\r\n\r\n";
static const int HEAD_END_SIZE=4;
static const char * const LINE_END="\r\n";
static const char *const CONTENT_LENGTH_KEY="Content-Length";
ProtocalWebsocket::ProtocalWebsocket(uint32_t _max_cache_size, uint32_t _max_frame_size, uint32_t _slice_size):ProtocalBase(_max_cache_size,true,_max_frame_size,_slice_size==0?1200:_slice_size)
{

}

ProtocalWebsocket::~ProtocalWebsocket()
{

}

std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalWebsocket::packetFrame(const void *payload,uint32_t payload_len,bool use_mask,WS_FrameType frame_type,bool is_fin)
{

    WSFrameHeader header;
    memset(&header,0,sizeof (header));
    header.FIN=is_fin;
    header.MASK=use_mask;
    header.opcode=(frame_type)&0xf;
    if(payload_len<126){
        header.payload_len|=payload_len;
        header.padding_payload_len=0;
    }
    else if (payload_len<=65536) {
        header.payload_len|=126;
        header.padding_payload_len=2;
        header.extended_payload_len.len_for_below_or_equal_65536|=payload_len;
    }
    else  {
        header.payload_len|=127;
        header.padding_payload_len=8;
        header.extended_payload_len.len_for_larger_than_65536|=payload_len;
    }
    if(header.MASK){
        WSFrameHeader::generate_random_mask_key(&header.Masking_key);
    }
    uint32_t final_buf_size=2/*base*/+header.padding_payload_len/*payload_len extended*/+(header.MASK?4:0)+payload_len;
    std::shared_ptr<uint8_t>final_buf(new uint8_t[final_buf_size+1],std::default_delete<uint8_t[]>());
    memset(final_buf.get(), 0, final_buf_size+1);
    // set fin and opcode
    uint32_t operation_pos=0;

    final_buf.get()[operation_pos]=static_cast<uint8_t>(header.opcode);
    if(header.FIN)final_buf.get()[operation_pos]|=0x80;
    operation_pos++;//FIN and opcode
    //set MASK
    if(header.MASK)final_buf.get()[operation_pos]|=0x80;
    //filled payload_len filed
    final_buf.get()[operation_pos]|=header.payload_len;

    if(header.payload_len<126){
        //just do nothing
    }
    else if (header.payload_len==126) {
        final_buf.get()[operation_pos+1]=(header.extended_payload_len.len_for_below_or_equal_65536>>8)&0xff;
        final_buf.get()[operation_pos+2]=(header.extended_payload_len.len_for_below_or_equal_65536>>0)&0xff;
    }
    else {
        final_buf.get()[operation_pos+1]=(header.extended_payload_len.len_for_larger_than_65536>>56)&0xff;
        final_buf.get()[operation_pos+2]=(header.extended_payload_len.len_for_larger_than_65536>>48)&0xff;
        final_buf.get()[operation_pos+3]=(header.extended_payload_len.len_for_larger_than_65536>>40)&0xff;
        final_buf.get()[operation_pos+4]=(header.extended_payload_len.len_for_larger_than_65536>>32)&0xff;
        final_buf.get()[operation_pos+5]=(header.extended_payload_len.len_for_larger_than_65536>>24)&0xff;
        final_buf.get()[operation_pos+6]=(header.extended_payload_len.len_for_larger_than_65536>>16)&0xff;
        final_buf.get()[operation_pos+7]=(header.extended_payload_len.len_for_larger_than_65536>>8)&0xff;
        final_buf.get()[operation_pos+8]=(header.extended_payload_len.len_for_larger_than_65536>>0)&0xff;
    }
    operation_pos++;//payload_len
    operation_pos+=header.padding_payload_len;//extended_payload_len
    //fill Masking_key
    if(header.MASK){
        final_buf.get()[operation_pos]=static_cast<uint8_t>(header.Masking_key[0]);
        final_buf.get()[operation_pos+1]=static_cast<uint8_t>(header.Masking_key[1]);
        final_buf.get()[operation_pos+2]=static_cast<uint8_t>(header.Masking_key[2]);
        final_buf.get()[operation_pos+3]=static_cast<uint8_t>(header.Masking_key[3]);
        operation_pos+=4;//Masking_key
    }
    //copy data
    if(payload_len>0)
    {
        memcpy(final_buf.get()+operation_pos,payload,payload_len);
    }
    //encode data with mask
    if(header.MASK){
        for(uint32_t i=0;i<payload_len;++i){
            final_buf.get()[i+operation_pos]^=header.Masking_key[i&0x3];
        }
    }
    return std::make_pair(final_buf,final_buf_size);
}


std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalWebsocket::encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list)
{
    (void)input_list;
    std::shared_ptr<uint8_t>ret_buf;
    uint32_t ret_len=0;
    return {ret_buf,ret_len};
}

std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> ProtocalWebsocket::decodeFrame(const void *data,uint32_t len)
{
    uint32_t frame_start=0;
    uint32_t frame_len=0;
    do{
        if(len<minProtocalSize())break;
        auto header=decodeFrameHeader(data,len);
        if(header.actual_header_len<0)break;
        auto need_size=header.actual_header_len+header.actual_payload_len;
        if(need_size>len)break;
        frame_len=need_size;
    }while(0);
    return {frame_start,frame_len};
}



WSFrameHeader ProtocalWebsocket::decodeFrameHeader(const void *data, uint32_t data_len)
{
    WSFrameHeader header;
    auto raw_buf=reinterpret_cast<const uint8_t *>(data);
    uint32_t decode_pos=0;
    header.FIN = (raw_buf[decode_pos] & 0x80) == 0x80;//get fin
    header.opcode=raw_buf[decode_pos]&0xf;//get opcode
    decode_pos++;
    header.MASK = (raw_buf[decode_pos] & 0x80) == 0x80; //  get mask
    header.payload_len = raw_buf[decode_pos] & 0x7F; // get payload_Len
    decode_pos++;
    uint32_t payload_len=0;
    do{
        if(header.payload_len <126){
            payload_len=static_cast<uint32_t>(header.payload_len);
        }
        else if (header.payload_len==126) {
            if(data_len<decode_pos+2)break;
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<8);
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<0);
        }
        else if(header.payload_len==127) {
            if(data_len<decode_pos+4)break;
            decode_pos++;//discard
            decode_pos++;//discard
            decode_pos++;//discard
            decode_pos++;//discard
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<24);
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<16);
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<8);
            payload_len|=static_cast<uint32_t>(raw_buf[decode_pos++]<<0);
        }
        else {
            //never execute
        }
        if(header.MASK)
        {
            if(data_len<decode_pos+4)break;
            header.Masking_key[0]=static_cast<char>(raw_buf[decode_pos++]);
            header.Masking_key[1]=static_cast<char>(raw_buf[decode_pos++]);
            header.Masking_key[2]=static_cast<char>(raw_buf[decode_pos++]);
            header.Masking_key[3]=static_cast<char>(raw_buf[decode_pos++]);
        }
        header.actual_header_len=decode_pos;
        header.actual_payload_len=payload_len;
    }while(0);
    return header;

}

bool ProtocalWebsocket::bufferToFrame(void *buf, uint32_t buf_len, StreamWebsocketFrame *frame)
{
#ifdef DEBUG
    assert(frame!=nullptr);
#endif
    bool ret=false;
    auto ptr=reinterpret_cast<const char *>(buf);
    do{
        if(buf_len<minProtocalSize())break;
        frame->header=decodeFrameHeader(ptr,buf_len);
        if(frame->header.actual_header_len<0)
        {
            AIMY_ERROR("false websocket paket");
            break;
        }
        auto need_size=frame->header.actual_header_len+frame->header.actual_payload_len;
        if(need_size>buf_len)
        {
            AIMY_ERROR("false http pakcet,header not complete");
            break;
        }
        frame->payload=reinterpret_cast<uint8_t *>(buf)+frame->header.actual_header_len;
        ret=true;
    }while(0);
    return ret;

}

void ProtocalWebsocket::dumpFrame(const std::string &tag,const StreamWebsocketFrame &frame)
{
    AIMY_DEBUG("%s dump[%d %d %d %d %d]",tag.c_str(),frame.header.FIN,frame.header.RSV,frame.header.MASK,frame.header.opcode,frame.header.actual_payload_len);
}