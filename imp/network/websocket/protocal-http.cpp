#include "protocal-http.h"
#include "core/core-include.h"
#include<cstring>
#include<algorithm>
using namespace aimy;
static const char *const  HEAD_END="\r\n\r\n";
static const int HEAD_END_SIZE=4;
static const char * const LINE_END="\r\n";
static const char *const CONTENT_LENGTH_KEY="Content-Length";
ProtocalHttp::ProtocalHttp(uint32_t _max_cache_size, uint32_t _max_frame_size, uint32_t _slice_size):ProtocalBase(_max_cache_size,true,_max_frame_size,_slice_size==0?1200:_slice_size)
{

}

ProtocalHttp::~ProtocalHttp()
{

}

std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalHttp::packetFrame(const std::vector<std::string> &header, std::map<std::string, std::string> &keys_map, const void * content, uint32_t content_len)
{

    std::list<std::pair<const void *,uint32_t>> input_list;
    std::string header_data;
    for(auto & i :header)
    {
        if(header_data.empty())header_data+=i;
        else {
            header_data+=" "+i;
        }
    }
    header_data+=LINE_END;
    if(content_len==0)
    {
        keys_map.erase(CONTENT_LENGTH_KEY);
    }
    else {
        keys_map[CONTENT_LENGTH_KEY]=std::to_string(content_len);
    }
    //add keys
    for(auto &i:keys_map)
    {
        header_data+=i.first+": "+i.second+LINE_END;
    }
    header_data+=LINE_END;
    auto ptr=header_data.c_str();
    input_list.push_back(std::make_pair(ptr,header_data.length()));
    if(content_len>0)input_list.push_back(std::make_pair(content,content_len));
    return encodeFrame(input_list);
}


std::pair<std::shared_ptr<uint8_t>,uint32_t> ProtocalHttp::encodeFrame(const std::list<std::pair<const void *,uint32_t>> &input_list)
{
    std::shared_ptr<uint8_t>ret_buf;
    uint32_t ret_len=0;
    uint32_t total_len=0;
    for(const auto &i:input_list)
    {
        total_len+=i.second;
    }
    do{
        if(input_list.size()<1)
        {
            AIMY_ERROR("false input for http encode");
            break;
        }
        if(total_len>getMaxFrameSize())break;
        ret_len=total_len;
        ret_buf.reset(new uint8_t[total_len+1],std::default_delete<uint8_t[]>());
        memset(ret_buf.get(),0,total_len+1);
        uint32_t offset=0;
        auto iter=input_list.begin();
        //header
        memcpy(ret_buf.get()+offset,iter->first,iter->second);
        offset+=iter->second;
        //content
        ++iter;
        if(iter!=input_list.end())memcpy(ret_buf.get()+offset,iter->first,iter->second);
    }while(0);
    return {ret_buf,ret_len};
}

std::pair<uint32_t/*frame start offset*/,uint32_t/*frame len*/> ProtocalHttp::decodeFrame(const void *data,uint32_t len)
{
    uint32_t frame_start=0;
    uint32_t frame_len=0;
    do{
        if(header_cache.process_finished)
        {
            frame_len=len;
            break;
        }
        if(len<minProtocalSize())break;
        //find \r\n\r\n
        const char *data_ptr=reinterpret_cast<const char *>(data);
        decodeFrameHeader(&header_cache,data_ptr,len);
        if(header_cache.parse_error)
        {
            frame_len=getMaxFrameSize()+len;
            break;
        }
        if(header_cache.content_length<0)break;
        auto need_size=header_cache.header_offset+header_cache.content_length;
        if(need_size>len)break;
        frame_len=need_size;
        header_cache.process_finished=true;
    }while(0);
    return {frame_start,frame_len};
}


const char * ProtocalHttp::getNextLine(const char *input)
{
    if(!input)return  nullptr;
    while(*input!='\0'){
        if(*input=='\r'||*input=='\n'){
        //find the end
            input++;
            if(*input=='\n') ++input;
            return input;
        }
        input++;
    }
    return  nullptr;
}
void ProtocalHttp::decodeFrameHeader(StreamHttpHeader *header, const char *data, uint32_t data_len)
{
    do{
        if(header->content_length>=0)break;
        auto tmp_pos=std::search(data,data+data_len,HEAD_END,HEAD_END+HEAD_END_SIZE);
        if(tmp_pos==data+data_len)break;//not found wait
        char a[4096]={0};
        char b[4096]={0};
        char c[4096]={0};
        if(sscanf(data,"%s %s %[^\r\n]",a,b,c)!=3)
        {
            header->parse_error=true;
            AIMY_ERROR("unknown http head:%s!",data);
            break;
        }
        const char *temp=data;
        while((temp=getNextLine(temp))!=nullptr&&temp<tmp_pos){
            char key[4096]={0};
            char value[4096]={0};
            if(sscanf(temp,"%[^:]: %[^\r\n]",key,value)!=2){
                header->keys.clear();
                header->parse_error=true;
                break;
            }
            header->keys.emplace(key,value);
        }
        if(!header->keys.empty()&&!header->parse_error)
        {
            header->header.push_back(a);
            header->header.push_back(b);
            header->header.push_back(c);
            auto iter=header->keys.find(CONTENT_LENGTH_KEY);
            if(iter!=header->keys.end())
            {
                header->content_length=std::stoul(iter->second);
            }
            else {
                header->content_length=0;
            }
            header->header_offset=tmp_pos-data+HEAD_END_SIZE;
        }
    }while(0);
}

bool ProtocalHttp::bufferToFrame(void *buf, uint32_t buf_len, StreamHttpFrame * frame)
{
#ifdef DEBUG
    assert(frame!=nullptr);
#endif
    bool ret=false;
    auto ptr=reinterpret_cast<const char *>(buf);
    do{
        if(buf_len<minProtocalSize())break;
        decodeFrameHeader(&(frame->header),ptr,buf_len);
        if(frame->header.parse_error)
        {
            AIMY_ERROR("false http paket");
            break;
        }
        if(frame->header.content_length<0){
            AIMY_ERROR("false http pakcet,header not complete");
            break;
        }
        auto need_size=frame->header.header_offset+frame->header.content_length;
        if(need_size>buf_len)
        {
            AIMY_ERROR("false http pakcet,header not complete");
            break;
        }
        frame->content=reinterpret_cast<uint8_t *>(buf)+frame->header.header_offset;
        ret=true;

    }while(0);
    return ret;

}
