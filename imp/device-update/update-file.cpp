#include "update-file.h"
#include "core-include.h"
#include "imp/common/iutils.h"
using namespace aimy;
#define AIMY_HEX_STRING_MAX_LEN 0x20
struct DeviceInfo {
    char version[AIMY_HEX_STRING_MAX_LEN];
    char product[AIMY_HEX_STRING_MAX_LEN];
    char date[AIMY_HEX_STRING_MAX_LEN];
};

UpdateFileContext::UpdateFileContext():type(AIMY_UNKNOWN_FILE),updateFilename(""),fp(nullptr)
  ,fileSize(0),vecDataLen(0),codeDataLen(0)
{

}

UpdateFileContext::~UpdateFileContext()
{
    deInit();
}

bool UpdateFileContext::init(const std::string&fileName,AimyUpdateFileType guessType)
{
    deInit();
    bool ret=false;
    do{
        if(fileName.empty())
        {
            AIMY_ERROR("%s init failed,file name is empty",__FUNCTION__);
            break;
        }
        updateFilename=fileName;
        type=guessFileType(updateFilename,guessType);
        if(type==AIMY_UNKNOWN_FILE)
        {
            AIMY_ERROR("%s file [%s] is in unsupported format!",__FUNCTION__,updateFilename.c_str());
            break;
        }

        fp=fopen(updateFilename.c_str(),"rb");
        if(!fp)
        {
            AIMY_ERROR("%s open file failed [%s]",__FUNCTION__,strerror(platform::getErrno()));
            break;
        }
        fseek(fp,0,SEEK_END);
        auto total_size=ftell(fp);
        if(total_size<=0)
        {
            AIMY_ERROR("%s file [%s] maybe something error,please check the file!",__FUNCTION__,updateFilename.c_str());
            break;
        }
        fileSize=static_cast<size_t>(total_size);

        ret=true;
    }while(0);
    if(!ret)deInit();
    return ret;
}

void UpdateFileContext::deInit()
{
    if(!updateFilename.empty())
    {
        AIMY_DEBUG("%s deinit [%s]",__FUNCTION__,updateFilename.c_str());
        if(fp)fclose(fp);
        fp=nullptr;
        type=AIMY_UNKNOWN_FILE;
        fileSize=0;
        vecData.reset();
        vecDataLen=0;
        codeData.reset();
        codeDataLen=0;
        hexContext.reset();
    }
}

bool UpdateFileContext::loadProgramData(uint32_t vec_start_offset,uint32_t vec_end_offset,uint32_t code_start_offset,uint32_t code_end_offset)
{
    AIMY_DEBUG("%s file_size %u vec_start:%08x vec_end:%08x code_start:%08x code_end:%08x",__FUNCTION__,fileSize,vec_start_offset,vec_end_offset,code_start_offset,code_end_offset);
    if(!fp||vec_end_offset<vec_start_offset||code_end_offset<code_start_offset)return false;
    vecDataLen=vec_end_offset-vec_start_offset;
    codeDataLen=code_end_offset-code_start_offset;
    if(vecDataLen==0&&codeDataLen==0)return false;
    vecData.reset(new uint8_t[vecDataLen+1],std::default_delete<uint8_t[]>());
    codeData.reset(new uint8_t[codeDataLen+1],std::default_delete<uint8_t[]>());
    bool ret=false;
    switch (type) {
    case AIMY_BIN_FILE:
    case AIMY_ENCRYPT_BIN_FILE:
    {
        if(vecDataLen>0)
        {
            fseek(fp,vec_start_offset,SEEK_SET);
            auto read_len=fread(vecData.get(),1,vecDataLen,fp);
            if(read_len<=0)
            {
                AIMY_ERROR("read normal file failed");
                break;
            }
            else {
                AIMY_DEBUG("read len %ld",read_len);
            }
        }
        if(type==AIMY_ENCRYPT_BIN_FILE)
        {//no read device info
            if(codeDataLen>fileSize-sizeof (DeviceInfo))
            {
                codeDataLen=fileSize-sizeof (DeviceInfo);
            }
        }
        if(codeDataLen>0)
        {//原始文件从0开始读取
            fseek(fp,0,SEEK_SET);
            auto read_len=fread(codeData.get(),1,codeDataLen,fp);
            if(read_len<=0)
            {
                AIMY_ERROR("read normal file failed");
                break;
            }
            else {
                AIMY_DEBUG("read len %ld",read_len);
            }
        }
        ret=true;
    }
        break;
    case AIMY_HEX_FILE:
    {
        if(vecDataLen>0)
        {
            auto read_len=hexContext.readData(vec_start_offset,vecData.get(),vecDataLen);
            if(read_len<=0)
            {
                AIMY_ERROR("read hex file failed");
                break;
            }
            else {
                AIMY_DEBUG("read len %ld",read_len);
            }
        }
        if(codeDataLen>0)
        {
            auto read_len=hexContext.readData(code_start_offset,codeData.get(),codeDataLen);
            if(read_len<=0)
            {
                AIMY_ERROR("read hex file failed");
                break;
            }
            else {
                AIMY_DEBUG("read len %ld",read_len);
            }
        }
        ret=true;
    }
        break;
    default:
        break;
    }
    if(!ret)
    {
        vecDataLen=0;
        codeDataLen=0;
        vecData.reset();
        codeData.reset();
    }
    return ret;
}

bool UpdateFileContext::fillUpdateSliceContext(UpdateSliceContext &context)
{
    if(context.max_slice_size<1){
        AIMY_FATALERROR("packet size must been larger than zero!");
        return false;
    }
    bool ret=true;
    do{
        //reset state
        memset(context.data.get(),0,context.max_slice_size);
        context.data_len=0;
        if(context.seq<-1)
        {//init
            //单片机分页存储 不能跨页写入，这里进行相应处理
            uint32_t page_left=context.page_size;
            //init vector map
            for(uint32_t i=0;i<vecDataLen;)
            {
                uint32_t max_len=vecDataLen-i;
                max_len=max_len>context.max_slice_size?context.max_slice_size:max_len;
                max_len=max_len>page_left?page_left:max_len;
                context.vector_map.push_back(std::make_pair(i,max_len));
                i+=max_len;
                page_left-=max_len;
                if(page_left==0)page_left=context.page_size;

            }
            //init code map
            for(uint32_t i=0;i<codeDataLen;)
            {
                uint32_t max_len=codeDataLen-i;
                max_len=max_len>context.max_slice_size?context.max_slice_size:max_len;
                max_len=max_len>page_left?page_left:max_len;
                context.code_map.push_back(std::make_pair(i,max_len));
                i+=max_len;
                page_left-=max_len;
                if(page_left==0)page_left=context.page_size;
            }
            if(!context.vector_map.empty())
            {
                context.seq=context.vector_map.size()-1;
                context.type=UPDATE_VECTOR_SLICE;
            }
            else {
                context.seq=context.code_map.size()-1;
                context.type=UPDATE_CODE_SLICE;
            }
        }
        if(context.seq==-1&&context.type==UPDATE_VECTOR_SLICE)
        {
            context.type=UPDATE_CODE_SLICE;
            context.seq=static_cast<int>(context.vector_map.size())-1;
        }
        if(context.type==UPDATE_CODE_SLICE&&context.seq<0)
        {//finished
            break;
        }
        //read data
        const uint8_t *opt_data_ptr=vecData.get();
        const std::vector<std::pair<uint32_t,uint32_t>> *offset_map=&(context.vector_map);
        if(context.type==UPDATE_CODE_SLICE)
        {
            opt_data_ptr=codeData.get();
            offset_map=&(context.code_map);
        }
        //check
        if(context.seq>=static_cast<int>(offset_map->size()))
        {
            AIMY_ERROR("packet read_offset error!");
            ret=false;
            break;
        }
        uint32_t index=offset_map->size()-1-context.seq;

        memcpy(context.data.get(),opt_data_ptr+(*offset_map)[index].first,(*offset_map)[index].second);
        //单片机那边要求数据四字节对齐
        if((*offset_map)[index].second&0x3)
        {
            context.data_len=((*offset_map)[index].second&0xfffffffc)+4;
        }
        else {
            context.data_len=(*offset_map)[index].second;
        }

    }while(0);
    return ret;
}

int UpdateFileContext::checkFile(const std::string&version,const std::string &product,const std::string&date,uint32_t version_offset,uint32_t product_offset,uint32_t date_offset)
{
    if(!fp)
    {
        AIMY_ERROR("not inited file!");
        return -1;
    }
    switch (type) {
    case AIMY_HEX_FILE:
        return checkHexFile(version,product,date,version_offset,product_offset,date_offset);
    case AIMY_BIN_FILE:
        return checkBinFile(version,product,date);
    case AIMY_ENCRYPT_BIN_FILE:
        return checkEncryptFile(version,product,date);
    case AIMY_UNKNOWN_FILE:
        break;
    }
    AIMY_ERROR("unknown file!");
    return -1;
}

AimyUpdateFileType UpdateFileContext::guessFileType(const std::string &file_name,AimyUpdateFileType guessType)
{

    auto ret=guessType;
    do{
        auto suffix=Iutils::getSuffix(file_name);
        if(suffix.empty())
        {
            AIMY_WARNNING("%s file[%s] is in false format",__FUNCTION__,file_name.c_str());
            break;
        }
        std::vector<std::string>match_list;
        match_list.push_back("hex");
        match_list.push_back("mva");
        match_list.push_back("amva");
        for(uint8_t i=AIMY_HEX_FILE;i<AIMY_UNKNOWN_FILE;++i)
        {
            if(strcasecmp(match_list[i].c_str(),suffix.c_str())==0)
            {
                AIMY_DEBUG("%s file's suffix match [%s]",__FUNCTION__,match_list[i].c_str());
                ret=static_cast<AimyUpdateFileType>(i);
                break;
            }
        }
    }while(0);
    return ret;
}

int UpdateFileContext::checkBinFile(const std::string&version,const std::string &product,const std::string&date)
{
    const uint8_t key[] = {
        'A', 'A', 'M', '_', 'P', 'R', 'O', 'D',
        'U', 'C', 'T', '_', 'I', 'N', 'F', 'O',
        'R', 'M', 'A', 'T', 'I', 'O', 'N', '\0',
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    };

#pragma pack (push,1)
    struct Information {
        char version[sizeof(key)];
        char product[sizeof(key)];
        char date[sizeof(key)];
    };
#pragma pack(pop)

    fseek(fp, 0, SEEK_SET);

    uint8_t buffer[sizeof(key) << 2];
    size_t offset = 0;

    for (;;) {
        size_t data_length = fread(buffer + offset, 1, sizeof(buffer) - offset, fp);
        data_length += offset;
        if (data_length < sizeof(buffer))
            return -1;

        size_t max_i = data_length - sizeof(key) + 1;
        for (size_t i = 0; i < max_i; ++i) {
            bool is_pass = true;
            for (size_t j = 0; j < sizeof(key); ++j) {
                if (buffer[i + j] != key[j]) {
                    is_pass = false;
                    break;
                }
            }

            if (is_pass) {
                size_t offset = data_length - i - sizeof(key);
                if (offset)
                    memmove(buffer, buffer + i + sizeof(key), offset);
                data_length = fread(buffer + offset, 1, sizeof(Information) - offset, fp);
                if (data_length + offset < sizeof(Information))
                    return -1;
                goto FOUND;
            }
        }

        offset = sizeof(key) - 1;
        memmove(buffer, buffer + max_i, offset);
    }

FOUND:
    const Information *p_info = (Information*)buffer;
    AIMY_DEBUG("version[%s->%s] product[%s->%s] date[%s->%s]",version.c_str(),p_info->version,product.c_str(),p_info->product,date.c_str(),p_info->date);
    if(product!=p_info->product){
        auto force_update=getenv("FORCE_UPDATE");
        if(force_update&&strcasecmp(force_update,"true")==0)
        {
            return 1;
        }
        AIMY_ERROR("product not match!");
        return -1;
    }
    if(version==p_info->version&&date==p_info->date)
    {
        AIMY_WARNNING("the same version");
        return 0;
    }
    return 1;
}

int UpdateFileContext::checkEncryptFile(const std::string&version,const std::string &product,const std::string&date)
{

    fseek(fp, -(long)sizeof(DeviceInfo), SEEK_END);

    DeviceInfo dev_info;
    if (fread(&dev_info, sizeof(dev_info), 1, fp) == 1) {
        AIMY_DEBUG("version[%s->%s] product[%s->%s] date[%s->%s]",version.c_str(),dev_info.version,product.c_str(),dev_info.product,date.c_str(),dev_info.date);
        if(product!=dev_info.product){
            auto force_update=getenv("FORCE_UPDATE");
            if(force_update&&strcasecmp(force_update,"true")==0)
            {
                return 1;
            }
            AIMY_ERROR("product not match!");
            return -1;
        }
        if(version==dev_info.version&&date==dev_info.date)
        {
            AIMY_WARNNING("the same version");
            return 0;
        }
        return 1;
    }
    AIMY_ERROR("file read failed[%s]",strerror(platform::getErrno()));
    return -1;
}

int UpdateFileContext::checkHexFile(const std::string&version,const std::string &product,const std::string&date,uint32_t version_offset,uint32_t product_offset,uint32_t date_offset)
{
    if(hexContext.loadHexFile(fp))
    {
        return -1;
    }
    DeviceInfo dev_info;
    if(hexContext.readData(version_offset,dev_info.version,AIMY_HEX_STRING_MAX_LEN)<=0)
    {
        AIMY_ERROR("hex read version failed!");
        return -1;
    }
    if(hexContext.readData(product_offset,dev_info.product,AIMY_HEX_STRING_MAX_LEN)<=0)
    {
        AIMY_ERROR("hex read product failed!");
        return -1;
    }
    if(hexContext.readData(date_offset,dev_info.date,AIMY_HEX_STRING_MAX_LEN)<=0)
    {
        AIMY_ERROR("hex read date failed!");
        return -1;
    }
    if(product!=dev_info.product){
        auto force_update=getenv("FORCE_UPDATE");
        if(force_update&&strcasecmp(force_update,"true")==0)
        {
            return 1;
        }
        AIMY_ERROR("product not match!");
        return -1;
    }
    AIMY_DEBUG("version[%s->%s] product[%s->%s] date[%s->%s]",version.c_str(),dev_info.version,product.c_str(),dev_info.product,date.c_str(),dev_info.date);
    if(version==dev_info.version&&date==dev_info.date)
    {
        AIMY_WARNNING("the same version");
        return 0;
    }
    return 1;
}
