#include "update-type-def.h"
#include "log/aimy-log.h"
#include<string.h>
#include<stdlib.h>
#if defined(__linux) || defined(__linux__)
#	include <endian.h>
#endif
using namespace aimy;
// :  1 byte
// len 2 byte
// address 4byte
// type 2 byte
// data n byte
// crc  2byte
// \r\n  2byte


int HexFileContext::readData(FILE *fp,uint32_t data_addr,void *buf,uint32_t& max_read_len)
{
    memset(buf,0,max_read_len);
    int ret=-1;
    uint32_t start_addr=data_addr;
    uint32_t end_addr=data_addr+max_read_len;
    std::shared_ptr<uint8_t> flag(new uint8_t[max_read_len],std::default_delete<uint8_t[]>());
    memset(flag.get(),0,max_read_len);
    uint32_t max_addr=0;
    do{
        if(!fp)break;
        fseek(fp,0,SEEK_SET);
        char read_cache[256]={0};
        char convert_box[5];
        //
        bool find_LINEAR_ADDR=false;
        uint32_t last_ex_linear_addr=0;
        while (1) {
            memset(read_cache,0,256);
            if(!fgets(read_cache,256,fp))break;
            auto data_len=strlen(read_cache);
            if(data_len<11||read_cache[0]!=':'){
                AIMY_ERROR("false hex file!");
                break;
            }
            if(read_cache[data_len-1]=='\n'){
                if(read_cache[data_len-2]=='\r'){
                    read_cache[data_len-2]=0;
                    data_len-=2;
                }
                else {
                    read_cache[data_len-1]=0;
                    --data_len;
                }
            }
            if(data_len<11){
                AIMY_ERROR("false hex file!");
                break;
            }
            uint32_t parse_offset=1;
            //check len
            convert_box[0]=read_cache[parse_offset++];
            convert_box[1]=read_cache[parse_offset++];
            convert_box[2]='\0';
            uint8_t payload_len=strtol(convert_box,nullptr,16)&0xff;
            uint32_t need_len=1+2+4+2+payload_len*2+2;
            if(data_len!=need_len){
                AIMY_ERROR("false hex file!len mismatch");
                break;
            }
            //read address
            convert_box[0]=read_cache[parse_offset++];
            convert_box[1]=read_cache[parse_offset++];
            convert_box[2]=read_cache[parse_offset++];
            convert_box[3]=read_cache[parse_offset++];
            convert_box[4]='\0';
            uint16_t offset_address=strtol(convert_box,nullptr,16)&0xffff;
            //read type
            convert_box[0]=read_cache[parse_offset++];
            convert_box[1]=read_cache[parse_offset++];
            convert_box[2]='\0';
            uint8_t type=strtol(convert_box,nullptr,16)&0xff;
            //read payload
            std::shared_ptr<uint8_t>tmp(new uint8_t[payload_len+1],std::default_delete<uint8_t[]>());
            memset(tmp.get(),0,payload_len+1);
            for(uint8_t i=0;i<payload_len;++i)
            {
                convert_box[0]=read_cache[parse_offset++];
                convert_box[1]=read_cache[parse_offset++];
                convert_box[2]='\0';
                tmp.get()[i]=strtol(convert_box,nullptr,16)&0xff;
            }
            //read crc
            convert_box[0]=read_cache[parse_offset++];
            convert_box[1]=read_cache[parse_offset++];
            convert_box[2]='\0';
            uint8_t check_code=strtol(convert_box,nullptr,16)&0xff;
            //check crc
            uint8_t sum=check_code+payload_len+(offset_address>>8)+(offset_address&0xff)+type;
            for(uint8_t i=0;i<payload_len;++i)sum+=tmp.get()[i];
            if(sum!=0)
            {
                AIMY_ERROR("hex record check failed!");
                break;
            }
            switch (type) {
            case RECORD_TYPE_DATA:{
                if(!find_LINEAR_ADDR)break;
                uint32_t data_start_addr=last_ex_linear_addr+offset_address;
                uint32_t data_end_addr=data_start_addr+payload_len;
                max_addr=data_end_addr>max_addr?data_end_addr:max_addr;
                if(data_start_addr<start_addr)
                {
                    if(data_end_addr<start_addr)
                    {
                        //miss
                    }
                    else {
                        ret=true;
                        uint32_t actual_end_addr=data_end_addr<end_addr?data_end_addr:end_addr;
                        uint32_t offset=start_addr-data_start_addr;
                        uint32_t copy_len=actual_end_addr-start_addr;
                        memcpy(buf,tmp.get()+offset,copy_len);
                        memset(flag.get(),1,copy_len);
                    }
                }
                else if (data_start_addr>=start_addr) {
                    if(data_start_addr>end_addr)
                    {
                        //miss
                    }
                    else {
                        ret=true;
                        uint32_t actual_end_addr=data_end_addr<end_addr?data_end_addr:end_addr;
                        uint32_t offset=data_start_addr-start_addr;
                        uint32_t copy_len=actual_end_addr-data_start_addr;
                        memcpy(reinterpret_cast<uint8_t *>(buf)+offset,tmp.get(),copy_len);
                        memset(flag.get()+offset,1,copy_len);
                    }
                }
            }
                break;
            case RECORD_TYPE_EOF:
                find_LINEAR_ADDR=false;
                break;
            case RECORD_TYPE_EX_LINEAR_ADDR:
            {
                if(payload_len!=2)
                {
                    AIMY_ERROR("hex addr payload error!");
                    break;
                }
                find_LINEAR_ADDR=true;
                AIMY_ERROR("%s",read_cache);
                last_ex_linear_addr=(uint32_t)be16toh(*(uint16_t*)tmp.get()) << 16;
                AIMY_ERROR("last_ex_linear_addr[%d] %08X",type,last_ex_linear_addr);
            }
                break;
            case RECORD_TYPE_START_LINEAR_ADDR:
            {
                if(payload_len!=4)
                {
                    AIMY_ERROR("hex addr payload error!");
                    break;;
                }
                find_LINEAR_ADDR=false;
                AIMY_ERROR("%s",read_cache);
                last_ex_linear_addr=be32toh(*(uint32_t*)tmp.get());
                AIMY_ERROR("last_ex_linear_addr[%d] %08X",type,last_ex_linear_addr);
            }
                break;
            case RECORD_TYPE_EX_SEG_ADDR:
            case RECORD_TYPE_START_SEG_ADDR:
                find_LINEAR_ADDR=false;
                AIMY_ERROR("not supported hex addr type");
                break;
            }
        }
    }while(0);
    uint32_t leng=max_read_len;
    max_read_len=0;
    for(uint32_t i=0;i<leng;++i)
    {
        if(flag.get()[i])++max_read_len;
        else {
            break;
        }
    }
    AIMY_ERROR("max_addr:%08X read_len:%u",max_addr,max_read_len);
    return ret;
}

