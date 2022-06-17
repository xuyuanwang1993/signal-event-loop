#ifndef CHANGELOGFORMAT_H
#define CHANGELOGFORMAT_H
#include<string>
#include<memory>
#include<map>
#include<list>
#include<vector>
namespace aimy {
class ChangelogFormatter{
    enum LineType:uint8_t{
        UNDEFINED_TYPE,
        HEADER_START,
        HEADER_END,
        SCOPE_COMMENT_START,
        SCOPE_COMMENT_END,
        LINE_COMMENT,
        VERSION_START,
        VERSION_END,
        TITLE_LEVEL_1,
        TITLE_LEVEL_2,
        DISCARD_CONTEXT,
    };
    struct version_info{
        std::string version;
        std::string time_str;
        uint32_t start_line=0;
        uint32_t end_line=0;
        version_info(const std::string &_verion,const std::string &time,uint32_t start,uint32_t end)
            :version(_verion),time_str(time),start_line(start),end_line(end){

        }
    };
    struct header_info{
        std::string descrpition;
        uint32_t start_line=0;
        uint32_t end_line=0;
    };

    struct line_data{
        LineType type;
        std::string data;
        line_data(LineType _type=UNDEFINED_TYPE,std::string _data=""):type(_type),data(_data){}
    };
    struct version_compare_less{
        std::vector<uint32_t> str_to_ver(const std::string &version)const
        {
            std::vector<uint32_t>ret;
            size_t len=version.size();
            size_t start_pos=0;
            size_t data_len=0;
            const char * data_ptr=version.c_str();
            while (start_pos+data_len<len) {
                if(data_ptr[start_pos+data_len]>='0'&&data_ptr[start_pos+data_len]<='9')
                {//num
                    ++data_len;
                    continue;
                }
                else if (data_ptr[start_pos+data_len]=='.') {
                    if(data_len>0)ret.push_back(std::stoul(version.substr(start_pos,data_len)));
                    else {
                        ret.push_back(0);
                    }
                    //handle
                    start_pos=start_pos+data_len+1;
                    data_len=0;
                }
                else {
                    //error
                    return {};
                }
            }
            if(data_len>0)ret.push_back(std::stoul(version.substr(start_pos,data_len)));
            return ret;
        }
        bool
        operator()(const std::string& __x, const std::string& __y) const
        {
            auto vec1=str_to_ver(__x);
            auto vec2=str_to_ver(__y);
            auto len=vec1.size()>vec2.size()?vec2.size():vec1.size();
            for(size_t i=0;i<len;++i)
            {
                if(vec1[i]<vec2[i])return true;
                else if (vec1[i]>vec2[i]) {
                    return false;
                }
            }
            return vec1.size()<vec2.size();
        }
    };
public:
    ChangelogFormatter();
    ~ChangelogFormatter();
    bool loadFile(const std::string &path);
    bool checkVersionExisted(const std::string &version);
    std::string getJsonVersionInfo(const std::string &version="");
    std::string getXmlVersionInfo(const std::string &version="");
    std::string getHtmlVersionInfo(const std::string &version="");
    bool dumpToFile(const std::string &path);
    static int handleCommandLine(int argc,char *argv[]);
    static void printHelpInfo();
    static bool isValidVersion(const std::string &version);
public:
    std::string htmlFormatContext(uint32_t start_index,uint32_t end_index);
    std::string getHtmlHeaderInfo();
private:
    static std::pair<const char *,uint32_t>trim(const char *data,uint32_t len);
    static bool isEmptyLine(const char*data,uint32_t len);
    static std::tuple<bool, uint32_t> isLineComment(const char *data,uint32_t len);
    static std::pair<bool,uint32_t> isHeaderStart(const char *data,uint32_t len);
    static bool isHeaderEnd(const char *data,uint32_t len);
    static bool isScopeCommentStart(const char *data,uint32_t len);
    static bool isScopeCommentEnd(const char *data,uint32_t len);
    static std::tuple<bool,uint32_t,uint32_t,uint32_t,uint32_t> isVersionStart(const char * data,uint32_t len);
    static bool isVersionEnd(const char * data,uint32_t len);
    static std::tuple<bool,std::string>isTitileLevel1(const char *data,uint32_t len);
    static std::tuple<bool,std::string>isTitileLevel2(const char *data,uint32_t len);
    static bool isTileMultiLine(const char *data,uint32_t len);
    static bool isDiscard(const char *data,uint32_t len);
    bool checkVersion(std::list<std::string>&cacheList,const std::string &verion,const std::string time);
    bool checkHeader(std::list<std::string>&cacheList,std::string description);
    std::map<uint32_t,line_data> contextMap;
    std::map<std::string,version_info,version_compare_less> versionsMap;
    header_info header;
};
}
#endif // CHANGELOGFORMAT_H
