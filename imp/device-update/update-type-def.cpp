#include "update-type-def.h"
#include "log/aimy-log.h"
#include<string.h>
#include<stdlib.h>
using namespace aimy;
int HexFileContext::readData(uint32_t data_addr,void *buf,uint32_t max_read_len)
{
    uint32_t base_addr=data_addr&AIMY_HEX_HIGH_MASK;
    uint32_t start_offset=data_addr&AIMY_HEX_LOW_MASK;
    int ret=-1;
    do{
        auto iter=cache_map.find(base_addr);
        if(iter==cache_map.end())
        {
            AIMY_ERROR("no hex data start with addr %08x",base_addr);
            break;
        }
        auto try_read_len=AIMY_HEX_LOW_MASK+1-start_offset;
        if(try_read_len>max_read_len)try_read_len=max_read_len;
        //read
        ret=0;
        auto &hexCache=*(iter->second);
        for(uint32_t i=start_offset;i<try_read_len;++i)
        {
            if(hexCache.data_flag[i])ret++;
        }
        if(ret>0)memcpy(buf,hexCache.data_cache+start_offset,ret);
    }while(0);
    return ret;
}
// :  1 byte
// len 2 byte
// address 4byte
// type 2 byte
// data n byte
// crc  2byte
// \r\n  2byte
bool HexFileContext::loadHexFile(FILE *fp)
{
    if(!fp)return false;
    char read_cache[256]={0};
    bool find_eof=false;
    char convert_box[5];
    //
    uint32_t last_ex_linear_addr=0;
    std::shared_ptr<AimyHexCache>last_cache=nullptr;
    while (1) {
        memset(read_cache,0,256);
        if(!fgets(read_cache,256,fp))break;
        auto data_len=strlen(read_cache);
        if(data_len<11||read_cache[0]!=':'){
            AIMY_ERROR("false hex file!");
            return false;
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
            return false;
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
            return false;
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
            return false;
        }
        switch (type) {
        case RECORD_TYPE_DATA:
            if(AIMY_HEX_LOW_MASK+1-offset_address<payload_len)
            {
                AIMY_ERROR("false hex file!");
                return false;
            }
            if(last_cache.get())
            {
                for(uint16_t i=offset_address;i<payload_len;++i)
                {
                    last_cache->data_flag[i]=1;
                }
                memcpy(last_cache->data_cache+offset_address,tmp.get(),payload_len);
            }
            break;
        case RECORD_TYPE_EOF:
            find_eof=true;
            goto FIND_EOF;
            break;
        case RECORD_TYPE_EX_LINEAR_ADDR:
        {
            if(payload_len!=2)
            {
                AIMY_ERROR("hex addr payload error!");
                return false;
            }
            last_ex_linear_addr=(uint32_t)be16toh(*(uint16_t*)tmp.get()) << 16;
            auto iter=cache_map.find(last_ex_linear_addr);
            if(iter==cache_map.end())
            {
               last_cache.reset(new AimyHexCache(last_ex_linear_addr));
               cache_map[last_ex_linear_addr]=last_cache;
            }
            else {
                last_cache=iter->second;
            }
        }
            break;
        case RECORD_TYPE_EX_SEG_ADDR:
        case RECORD_TYPE_START_SEG_ADDR:
        case RECORD_TYPE_START_LINEAR_ADDR:
            last_cache.reset();
            AIMY_ERROR("not supported hex addr type");
            break;

        }
    }
FIND_EOF:
    return find_eof;
}

void HexFileContext::reset()
{
    cache_map.clear();
}
