#include "changelog-format.h"
#include "core/core-include.h"
#include "third_party/json/cjson-interface.h"
#include<string.h>
using namespace aimy;
ChangelogFormatter::ChangelogFormatter()
{

}

ChangelogFormatter::~ChangelogFormatter()
{
    contextMap.clear();
    versionsMap.clear();
}

bool ChangelogFormatter::isValidVersion(const std::string &version)
{
    //major.minor.iter
    do{
        auto pos1=version.find('.');
        if(pos1==std::string::npos||pos1==0)break;
        auto pos2=version.find('.',pos1+1);
        if(pos2==std::string::npos||pos2==pos1+1||pos2==version.size()-1)break;
        for(decltype (pos1)i=0;i<pos1;++i)
        {
            if(version[i]<'0'||version[i]>'9')goto  VERSION_FAILED;
        }
        for(decltype (pos1)i=pos1+1;i<pos2;++i)
        {
            if(version[i]<'0'||version[i]>'9')goto  VERSION_FAILED;
        }
        for(decltype (pos1)i=pos2+1;i<version.size();++i)
        {
            if(version[i]<'0'||version[i]>'9')goto  VERSION_FAILED;
        }
        {
            auto major=std::stoi(version.substr(0,pos1));
            auto minor=std::stoi(version.substr(pos1+1,pos2));
            auto iter=std::stoi(version.substr(pos2+1));
            fprintf(stderr,"%d.%d.%d\r\n",major,minor,iter);
        }
        return true;
    }while(0);
VERSION_FAILED:
    return false;
}

bool ChangelogFormatter::loadFile(const std::string &path)
{
    contextMap.clear();
    versionsMap.clear();
    header.descrpition.clear();
    FILE *fp=nullptr;
    bool ret_flag=false;
    do{
        fp=fopen(path.c_str(),"r");
        if(!fp)
        {
            fprintf(stderr,"open %s failed[%s]!",path.c_str(),strerror(platform::getErrno()));
            break;
        }
        ret_flag=true;
        size_t max_len=4096;
        std::shared_ptr<char> buf(new char[max_len+1],std::default_delete<char[]>());
        memset(buf.get(),0,max_len+1);
        char *ptr=buf.get();
        ssize_t read_len=0;
        while((read_len=getline(&ptr,&max_len,fp))!=-1)
        {
            auto ret=trim(ptr,static_cast<uint32_t>(read_len));
            //
            if(isEmptyLine(ret.first,ret.second))
            {
                memset(buf.get(),0,4097);
                continue;
            }
            //header handle
            if(contextMap.empty())
            {
                std::list<std::string>cachelist;
                auto ret1=isHeaderStart(ret.first,ret.second);
                if(ret1.first)
                {
                    std::string description=std::string(ret.first+ret1.second,ret.second-ret1.second);
                    bool find=false;
                    while((read_len=getline(&ptr,&max_len,fp))!=-1)
                    {
                        auto version_fix=trim(ptr,static_cast<uint32_t>(read_len));
                        auto check_ret=isHeaderEnd(version_fix.first,version_fix.second);
                        if(check_ret)
                        {
                            find=true;
                            break;
                        }
                        if(!isEmptyLine(version_fix.first,version_fix.second))
                        {
                            cachelist.push_back(std::string(version_fix.first,version_fix.second));
                        }
                        memset(buf.get(),0,4097);
                    }
                    if(!find)
                    {
                        fprintf(stderr,"find header end failed\r\n");
                        ret_flag=false;
                        break;
                    }
                    if(!checkHeader(cachelist,description))
                    {
                        fprintf(stderr,"header check failed\r\n");
                        ret_flag=false;
                    }
                    memset(buf.get(),0,4097);
                    continue;
                }
            }
            //discard handle
            if(isDiscard(ret.first,ret.second))
            {
                contextMap.emplace(contextMap.size(),line_data{DISCARD_CONTEXT});
                while((read_len=getline(&ptr,&max_len,fp))!=-1)
                {
                    contextMap.rbegin()->second.data.append(ptr,read_len);
                    memset(buf.get(),0,4097);
                }
                memset(buf.get(),0,4097);
                continue;
            }
            //line comment handle
            auto ret2=isLineComment(ret.first,ret.second);
            if(std::get<0>(ret2))
            {
                contextMap.emplace(contextMap.size(),line_data(LINE_COMMENT,std::string(ret.first+std::get<1>(ret2),ret.second-std::get<1>(ret2))));
                memset(buf.get(),0,4097);
                continue;
            }
            //scope comment handle
            auto ret3=isScopeCommentStart(ret.first,ret.second);
            if(ret3)
            {
                std::list<std::string>cachelist;
                bool find=false;
                memset(buf.get(),0,4097);
                while((read_len=getline(&ptr,&max_len,fp))!=-1)
                {
                    auto comment_fix=trim(ptr,static_cast<uint32_t>(read_len));
                    auto check_ret=isScopeCommentEnd(comment_fix.first,comment_fix.second);
                    if(check_ret)
                    {
                        find=true;
                        break;
                    }
                    if(!isEmptyLine(comment_fix.first,comment_fix.second))
                    cachelist.push_back(std::string(comment_fix.first,comment_fix.second));
                    memset(buf.get(),0,4097);
                }
                if(!find)
                {
                    fprintf(stderr,"find scope comment end failed\r\n");
                    ret_flag=false;
                    break;
                }
                if(!cachelist.empty())
                {
                    line_data temp(SCOPE_COMMENT_START);
                    for(auto i:cachelist)
                    {
                        if(!temp.data.empty())temp.data.append("\r\n");
                        temp.data.append(i);
                    }
                    contextMap.emplace(contextMap.size(),temp);
                    contextMap.emplace(contextMap.size(),line_data{SCOPE_COMMENT_END});
                }
                memset(buf.get(),0,4097);
                continue;
            }
            auto ret4=isVersionStart(ret.first,ret.second);
            //version handle
            if(std::get<0>(ret4))
            {
                std::list<std::string>cachelist;
                auto ver_p_1=std::get<1>(ret4);
                auto ver_p_2=std::get<2>(ret4);
                auto tim_p_1=std::get<3>(ret4);
                auto tim_p_2=std::get<4>(ret4);
                std::string ver(ptr+ver_p_1,ver_p_2-ver_p_1);
                std::string tmp1(ptr+tim_p_1,tim_p_2-tim_p_1);
                bool find=false;
                memset(buf.get(),0,4097);
                while((read_len=getline(&ptr,&max_len,fp))!=-1)
                {
                    auto version_fix=trim(ptr,static_cast<uint32_t>(read_len));
                    auto check_ret=isVersionEnd(version_fix.first,version_fix.second);
                    if(check_ret)
                    {
                        find=true;
                        break;
                    }
                    if(!isEmptyLine(version_fix.first,version_fix.second))
                    {
                        cachelist.push_back(std::string(version_fix.first,version_fix.second));
                    }
                    memset(buf.get(),0,4097);
                }
                if(!find)
                {
                    fprintf(stderr,"find version end failed\r\n");
                    ret_flag=false;
                    break;
                }
                if(!checkVersion(cachelist,ver,tmp1))
                {
                    fprintf(stderr,"version check failed\r\n");
                    ret_flag=false;
                }
                memset(buf.get(),0,4097);
                continue;
            }
            fprintf(stderr,"unknown rule!\r\n");
            ret_flag=false;//出现了不在解析规则中的项
            memset(buf.get(),0,4097);
        }

    }while(0);
    if(fp)fclose(fp);
    return ret_flag;
}

bool ChangelogFormatter::checkVersionExisted(const std::string &version)
{
    return versionsMap.find(version)!=versionsMap.end();
}

std::string ChangelogFormatter::getJsonVersionInfo(const std::string &version)
{
    auto concovert_func=[this](const version_info &info,const std::string &version)->neb::CJsonObject{
        neb::CJsonObject object;
        object.Add("version",version);
        object.Add("time",info.time_str);
        auto begin_iter=contextMap.find(info.start_line);
        auto end=info.end_line;
        neb::CJsonObject lvl1;
        for(auto i=begin_iter;i!=contextMap.end()&&i->first<end;)
        {
            auto iter=i;
            //find level 1
            ++i;
            if(iter->second.type!=TITLE_LEVEL_1)
            {
                continue;
            }
            neb::CJsonObject lvl1_obj;
            lvl1_obj.Add("level1",iter->second.data);
            neb::CJsonObject lvl2;
            for(;i!=contextMap.end()&&i->first<end;)
            {
                auto iter2=i;
                //find level 2
                if(iter2->second.type==TITLE_LEVEL_1)
                {
                    break;
                }
                ++i;
                if(iter2->second.type!=TITLE_LEVEL_2)
                {
                    continue;
                }
                lvl2.Add(iter2->second.data);
            }
            if(!lvl2.IsEmpty())lvl1_obj.Add("lvl2",lvl2);
            lvl1.Add(lvl1_obj);
        }
        object.Add("lvl1",lvl1);
        return object;
    };
    if(version.empty())
    {
        if(versionsMap.empty())return "";
        neb::CJsonObject object;
        for(auto iter=versionsMap.rbegin();iter!=versionsMap.rend();++iter)
        {
            object.Add(concovert_func(iter->second,iter->first));
        }
        return object.ToFormattedString();
    }
    auto iter=versionsMap.find(version);
    if(iter!=versionsMap.end())
    {
        return concovert_func(iter->second,iter->first).ToFormattedString();
    }
    else {
        return "";
    }
}

std::string ChangelogFormatter::getXmlVersionInfo(const std::string &version)
{
    auto concovert_func=[this](const version_info &info,const std::string &version)->std::string{
        std::string ret;
        ret+=std::string("<h2>Version ")+version+",Date "+info.time_str+"</h2>\r\n";
        ret+=htmlFormatContext(info.start_line,info.end_line);
        return ret;
    };
    std::string ret="<html>\r\n"
            "<head>\r\n"
            "<Title>Changelog History</Title>\r\n"
            "<meta charset=\"utf-8\">\r\n"
            "</head>\r\n"
            "<body >\r\n"
            "<h1>Changelog History</h1>\r\n";
    if(version.empty())
    {
        ret+=getHtmlHeaderInfo();
        if(versionsMap.empty())return "";
        for(auto iter=versionsMap.rbegin();iter!=versionsMap.rend();++iter)
        {
            ret+=concovert_func(iter->second,iter->first);
        }
    }
    else {
        auto iter=versionsMap.find(version);
        if(iter!=versionsMap.end())
        {
            ret+=concovert_func(iter->second,iter->first);
        }
        else {
            return "";
        }
    }
    ret+="</body>\r\n"
         "</html>\r\n";
    return ret;
}

std::string ChangelogFormatter::getHtmlVersionInfo(const std::string &version)
{
    auto concovert_func=[this](const version_info &info,const std::string &version)->std::string{
        std::string ret;
        ret+=std::string("<h2>Version ")+version+",Date "+info.time_str+"</h2>\r\n";
        ret+=htmlFormatContext(info.start_line,info.end_line);
        return ret;
    };
    std::string ret="<html>\r\n"
            "<head>\r\n"
            "<Title>Changelog History</Title>\r\n"
            "<meta charset=\"utf-8\">\r\n"
            "</head>\r\n"
            "<body >\r\n"
            "<h1>Changelog History</h1>\r\n";
    if(version.empty())
    {
        ret+=getHtmlHeaderInfo();
        if(versionsMap.empty())return "";
        for(auto iter=versionsMap.rbegin();iter!=versionsMap.rend();++iter)
        {
            ret+=concovert_func(iter->second,iter->first);
        }
    }
    else {
        auto iter=versionsMap.find(version);
        if(iter!=versionsMap.end())
        {
            ret+=concovert_func(iter->second,iter->first);
        }
        else {
            return "";
        }
    }
    ret+="</body>\r\n"
         "</html>\r\n";
    return ret;
}

std::string ChangelogFormatter::htmlFormatContext(uint32_t start_index,uint32_t end_index)
{
    std::string ret;
    auto begin_iter=contextMap.find(start_index);
    auto end=end_index;
    for(auto i=begin_iter;i!=contextMap.end()&&i->first<end;)
    {
        auto iter=i;
        //find level 1
        ++i;
        if(iter->second.type!=TITLE_LEVEL_1)
        {
            continue;
        }
        ret+=std::string("<h3>")+iter->second.data+"</h3>\r\n";
        std::list<std::string>title2_list;
        for(;i!=contextMap.end()&&i->first<end;)
        {
            auto iter2=i;
            //find level 2
            if(iter2->second.type==TITLE_LEVEL_1)
            {
                break;
            }
            ++i;
            if(iter2->second.type!=TITLE_LEVEL_2)
            {
                continue;
            }
            title2_list.push_back(std::string("<li>")+iter2->second.data+"\r\n");
        }
        if(!title2_list.empty())
        {
            ret+="<ul>\r\n";
            for(auto i:title2_list)
            {
                 ret+="\t"+i;
            }
            ret+="</ul>\r\n";
        }
    }
    return ret;
}

std::string ChangelogFormatter::getHtmlHeaderInfo()
{
    std::string ret;
    if(!header.descrpition.empty())
    {
        ret+=std::string("<h2>")+header.descrpition+"</h2>\r\n";
        ret+=htmlFormatContext(header.start_line,header.end_line);
    }
    return ret;
}

bool ChangelogFormatter::dumpToFile(const std::string &path)
{
    FILE *fp=nullptr;
    bool ret=false;
    do{
        if(!path.empty())
        {
            fp=fopen(path.c_str(),"w+");
            if(!fp)
            {
                fprintf(stderr,"open %s failed[%s]",path.c_str(),strerror(platform::getErrno()));
                break;
            }
        }
        for(auto i:contextMap)
        {
            switch (i.second.type) {
            case UNDEFINED_TYPE:
                break;
            case HEADER_START:
                fprintf(stdout,"$$%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"$$%s\r\n",i.second.data.c_str());
                break;
            case HEADER_END:
                fprintf(stdout,"$$$$\r\n\r\n");
                if(fp)fprintf(fp,"$$$$\r\n\r\n");
                break;
            case SCOPE_COMMENT_START:
                fprintf(stdout,"/*\r\n%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"/*\r\n%s\r\n",i.second.data.c_str());
                break;
            case SCOPE_COMMENT_END:
                fprintf(stdout,"*/\r\n\r\n");
                if(fp)fprintf(fp,"*/\r\n\r\n");
                break;
            case LINE_COMMENT:
                fprintf(stdout,"//%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"//%s\r\n",i.second.data.c_str());
                break;
            case VERSION_START:
                fprintf(stdout,"########%s########\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"########%s########\r\n",i.second.data.c_str());
                break;
            case VERSION_END:
                fprintf(stdout,"################################\r\n\r\n");
                if(fp)fprintf(fp,"################################\r\n\r\n");
                break;
            case TITLE_LEVEL_1:
                fprintf(stdout,"^%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"^%s\r\n",i.second.data.c_str());
                break;
            case TITLE_LEVEL_2:
                fprintf(stdout," **%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp," **%s\r\n",i.second.data.c_str());
                break;
            case DISCARD_CONTEXT:
                fprintf(stdout,"$\r\n%s\r\n",i.second.data.c_str());
                if(fp)fprintf(fp,"$\r\n%s",i.second.data.c_str());
                break;
            default:
                break;
            }
            if(i.second.type==DISCARD_CONTEXT)break;
        }
        ret=true;
    }while(0);
    if(fp)fclose(fp);
    return ret;
}

bool ChangelogFormatter::isEmptyLine(const char*data,uint32_t len)
{
    if(len==0)return true;
    if(data[0]=='\r'||data[0]=='\n')return true;
    return false;
}

std::pair<const char *,uint32_t> ChangelogFormatter::trim(const char *data,uint32_t len)
{
    auto ptr=data;
    while(len>0&&(ptr[0]==' '||ptr[0]=='\t'))
    {
        ++ptr;
        len--;
    }
    while(len>0&&(ptr[len-1]==' '||ptr[len-1]=='\t'||ptr[len-1]=='\r'||ptr[len-1]=='\n'))
    {
        --len;
    }
    return std::make_pair(ptr,len);
}

std::tuple<bool,uint32_t> ChangelogFormatter::isLineComment(const char *data,uint32_t len)
{
    if(len<2)return {false,0};
    auto ret=false;
    auto ret_pos=0;
    if(data[0]=='/'&&data[1]=='/')
    {
        ret=true;
        auto fix_ret=trim(data+2,len-2);
        ret_pos=fix_ret.first-data;
    }
    return {ret,ret_pos};
}

bool ChangelogFormatter::isScopeCommentStart(const char *data,uint32_t len)
{
    if(len<2)return false;
    bool ret=false;
    if(data[0]=='/'&&data[1]=='*')ret=true;
    return ret;
}

bool ChangelogFormatter::isScopeCommentEnd(const char *data,uint32_t len)
{
    if(len<2)return false;
    bool ret=false;
    if(data[0]=='*'&&data[1]=='/')ret=true;
    return ret;
}

std::pair<bool,uint32_t> ChangelogFormatter::isHeaderStart(const char *data,uint32_t len)
{
    if(len<2)return {false,0};
    auto ret=false;
    auto ret_pos=0;
    if(data[0]=='$'&&data[1]=='$')
    {
        ret=true;
        auto fix_ret=trim(data+2,len-2);
        ret_pos=fix_ret.first-data;
    }
    return {ret,ret_pos};
}

bool ChangelogFormatter::isHeaderEnd(const char *data,uint32_t len)
{
    if(len<2)return false;
    bool ret=false;
    if(data[0]=='$'&&data[1]=='$'&&data[2]=='$'&&data[3]=='$')ret=true;
    return ret;
}

std::tuple<bool, uint32_t, uint32_t, uint32_t, uint32_t> ChangelogFormatter::isVersionStart(const char * data,uint32_t len)
{
    bool ret1=false;
    uint32_t version_pos1=0;
    uint32_t version_pos2=0;
    uint32_t time_pos1=0;
    uint32_t time_pos2=0;
    do{
        if(len<20)break;
        bool failed=false;
        for(uint32_t i=0;i<8;++i)
        {
            if(data[i]!='#'){
                failed=true;
                break;
            }
        }
        if(failed)break;
        uint32_t pos_left_comma=0;
        for(uint32_t i=8;i<len;++i)
        {
            if(data[i]=='(')
            {
                pos_left_comma=i;
                break;
            }
        }
        if(pos_left_comma<=8)break;
        version_pos1=8;
        version_pos2=pos_left_comma;
        uint32_t pos_right_comma=0;
        for(uint32_t i=pos_left_comma;i<len;++i)
        {
            if(data[i]==')')
            {
                pos_right_comma=i;
                break;
            }
        }
        if(pos_right_comma<=pos_left_comma+1||pos_right_comma+8>=len)break;
        time_pos1=pos_left_comma+1;
        time_pos2=pos_right_comma;
        for(uint32_t i=1;i<9;++i)
        {
            if(data[pos_right_comma+i]!='#'){
                failed=true;
                break;
            }
        }
        if(failed)break;
        ret1=true;
    }while(0);
    return {ret1,version_pos1,version_pos2,time_pos1,time_pos2};
}

bool ChangelogFormatter::isVersionEnd(const char * data,uint32_t len)
{
    if(len<16)return false;
    bool success=true;
    for(uint32_t i=0;i<16;++i)
    {
        if(data[i]!='#')
        {
            success=false;
            break;
        }
    }
    return success;
}

std::tuple<bool,std::string> ChangelogFormatter::isTitileLevel1(const char *data,uint32_t len)
{
    if(len<2)return {false,""};
    auto ret=false;
    std::string ret_str;
    if(data[0]=='^')
    {
        ret=true;
        auto fix_ret=trim(data+1,len-1);
        if(ret==0)ret=false;
        else {
            ret_str=std::string(fix_ret.first,fix_ret.second);
        }
    }
    return {ret,ret_str};
}

std::tuple<bool,std::string> ChangelogFormatter::isTitileLevel2(const char *data,uint32_t len)
{
    if(len<2)return {false,""};
    auto ret=false;
    std::string ret_str;
    if(data[0]=='*'&&data[1]=='*')
    {
        ret=true;
        auto fix_ret=trim(data+2,len-2);
        if(ret==0)ret=false;
        else {
            ret_str=std::string(fix_ret.first,fix_ret.second);
        }
    }
    return {ret,ret_str};
}

bool ChangelogFormatter::isTileMultiLine(const char *data,uint32_t len)
{
    if(len==0)return true;
    if(data[0]=='$'||data[0]=='^'||data[0]=='*'||data[0]=='/'||data[0]=='#')return false;
    return true;
}

bool ChangelogFormatter::isDiscard(const char *data,uint32_t len)
{
    if(len<1)return false;
    return data[0]=='$';
}

bool ChangelogFormatter::checkVersion(std::list<std::string>&cacheList,const std::string &verion,const std::string time)
{
    std::list<line_data> cacheContextList;
    version_info ver(verion,time,contextMap.size()+1,0);
    cacheContextList.push_back(line_data{VERSION_START,verion+"("+time+")"});
    for(auto iter=cacheList.begin();iter!=cacheList.end();)
    {
        auto check_ret=isTitileLevel1(iter->c_str(),iter->size());
        if(std::get<0>(check_ret))
        {
            cacheContextList.push_back(line_data{TITLE_LEVEL_1,std::get<1>(check_ret)});
            ++iter;
            //apend title multi line
            for(;iter!=cacheList.end();)
            {
                if(isTileMultiLine(iter->c_str(),iter->size()))
                {
                    cacheContextList.rbegin()->data.append(std::string("\r\n")+*iter);
                    ++iter;
                }
                else {
                    break;
                }
            }
            for(;iter!=cacheList.end();)
            {
                auto check_ret1=isTitileLevel1(iter->c_str(),iter->size());
                if(std::get<0>(check_ret1))break;//find level1
                auto check_ret2=isTitileLevel2(iter->c_str(),iter->size());
                if(std::get<0>(check_ret2))
                {
                    cacheContextList.push_back(line_data{TITLE_LEVEL_2,std::get<1>(check_ret2)});
                    ++iter;
                    //apend title multi line
                    for(;iter!=cacheList.end();)
                    {
                        if(isTileMultiLine(iter->c_str(),iter->size()))
                        {
                            cacheContextList.rbegin()->data.append(std::string("\r\n")+*iter);
                            ++iter;
                        }
                        else {
                            break;
                        }
                    }
                }
                else {
                    fprintf(stderr,"%s is in flase format,need title level 2\r\n",iter->c_str());
                    goto VERSION_ERROR;
                }
            }
        }
        else {
            fprintf(stderr,"%s is in flase format,need title level 1\r\n",iter->c_str());
            goto VERSION_ERROR;
        }
    }
    cacheContextList.push_back(line_data{VERSION_END});
    for(auto i:cacheContextList)
    {
        contextMap.emplace(contextMap.size(),i);
    }
    ver.end_line=contextMap.size();
    versionsMap.emplace(verion,ver);
    return true;
VERSION_ERROR:
    return false;
}

bool ChangelogFormatter::checkHeader(std::list<std::string>&cacheList, std::string description)
{
    std::list<line_data> cacheContextList;
    header.descrpition=description;
    header.start_line=contextMap.size()+1;
    cacheContextList.push_back(line_data{HEADER_START,description});
    header.end_line=0;
    for(auto iter=cacheList.begin();iter!=cacheList.end();)
    {
        auto check_ret=isTitileLevel1(iter->c_str(),iter->size());
        if(std::get<0>(check_ret))
        {
            cacheContextList.push_back(line_data{TITLE_LEVEL_1,std::get<1>(check_ret)});
            ++iter;
            //apend title multi line
            for(;iter!=cacheList.end();)
            {
                if(isTileMultiLine(iter->c_str(),iter->size()))
                {
                    cacheContextList.rbegin()->data.append(std::string("\r\n")+*iter);
                    ++iter;
                }
                else {
                    break;
                }
            }
            for(;iter!=cacheList.end();)
            {
                auto check_ret1=isTitileLevel1(iter->c_str(),iter->size());
                if(std::get<0>(check_ret1))break;//find level1
                auto check_ret2=isTitileLevel2(iter->c_str(),iter->size());
                if(std::get<0>(check_ret2))
                {
                    cacheContextList.push_back(line_data{TITLE_LEVEL_2,std::get<1>(check_ret2)});
                    ++iter;
                    //apend title multi line
                    for(;iter!=cacheList.end();)
                    {
                        if(isTileMultiLine(iter->c_str(),iter->size()))
                        {
                            cacheContextList.rbegin()->data.append(std::string("\r\n")+*iter);
                            ++iter;
                        }
                        else {
                            break;
                        }
                    }
                }
                else {
                    fprintf(stderr,"%s is in flase format,need title level 2\r\n",iter->c_str());
                    goto HEADER_ERROR;
                }
            }
        }
        else {
            fprintf(stderr,"%s is in flase format,need title level 1\r\n",iter->c_str());
            goto HEADER_ERROR;
        }
    }
    cacheContextList.push_back(line_data{HEADER_END});
    for(auto i:cacheContextList)
    {
        contextMap.emplace(contextMap.size(),i);
    }
    header.end_line=contextMap.size();
    return true;
HEADER_ERROR:
    return false;
}

int ChangelogFormatter::handleCommandLine(int argc,char *argv[])
{
    if(argc<2)
    {
        printHelpInfo();
        return -1;
    }
    std::string file_name;
    std::string mode;
    std::string mode_option;
    std::string version;
    std::string dump_file;
    --argc;
    ++argv;
    while(argc>0)
    {
        std::string param(argv[0]);
        if(param=="-f")
        {
            --argc;
            ++argv;
            if(argc<=0)
            {
                fprintf(stderr,"need a file name\r\n");
                return -1;
            }
            file_name=argv[0];
        }
        else if (param=="-h"||param=="--help") {
            printHelpInfo();
        }
        else if (param=="-s") {
            --argc;
            ++argv;
            if(argc<=0)
            {
                fprintf(stderr,"need a version\r\n");
                return -1;
            }
            version=argv[0];
            if(!isValidVersion(version))
            {
                fprintf(stderr,"invalid version:%s\r\n",version.c_str());
                return -1;
            }
        }
        else if (param=="-d") {
            --argc;
            ++argv;
            if(argc<=0)
            {
                fprintf(stderr,"need a save file name\r\n");
                return -1;
            }
            dump_file=argv[0];
        }
        else {
            --argc;
            ++argv;
            mode=param;
            if(mode=="-o")
            {
                if(argc>0)
                {
                    mode_option=argv[0];
                }

            }
            else if (mode=="-c") {
                if(argc<=0)
                {
                    fprintf(stderr,"need a check version\r\n");
                    return -1;
                }
                mode_option=argv[0];
                if(!isValidVersion(mode_option))
                {
                    fprintf(stderr,"invalid version:%s\r\n",mode_option.c_str());
                    return -1;
                }
            }
            else if (mode=="-x") {
                if(argc<=0)
                {
                    fprintf(stderr,"need a output format [json/xml/html]\r\n");
                    return -1;
                }
                mode_option=argv[0];
            }
            else {
                fprintf(stderr,"false option %s\r\n",mode.c_str());
                return -1;
            }
        }
        --argc;
        ++argv;
    }
    if(file_name.empty())
    {
        fprintf(stderr,"need a file name\r\n");
        return -1;
    }
    if(mode.empty())
    {
        printHelpInfo();
        return -1;
    }
    ChangelogFormatter format;
    bool ret=format.loadFile(file_name);
    if(!ret)
    {
        fprintf(stderr,"file %s may be false format!\r\n",file_name.c_str());
    }
    if(mode=="-c")
    {
        bool hasVerion=format.checkVersionExisted(mode_option);
        if(!hasVerion)
        {
            fprintf(stderr,"version %s is not existed!\r\n",mode_option.c_str());
            return -1;
        }
        else {
            fprintf(stderr,"version %s is existed!\r\n",mode_option.c_str());
            return 0;
        }
    }
    else if (mode=="-o") {
        auto success=format.dumpToFile(mode_option);
        if(!success)
        {
            fprintf(stderr,"dump to  %s failed!\r\n",mode_option.c_str());
            return -1;
        }
        else {
            fprintf(stderr,"dump to  %s success!\r\n",mode_option.empty()?"stdout":mode_option.c_str());
            return 0;
        }
    }
    else if (mode=="-x") {
        std::string info;
        if(mode_option=="json")
        {
            info=format.getJsonVersionInfo(version);
        }
        else if (mode_option=="html") {
            info=format.getHtmlVersionInfo(version);
        }
        else if (mode_option=="xml") {
            info=format.getXmlVersionInfo(version);
        }
        else {
            fprintf(stderr,"not supported option %s!\r\n",mode_option.c_str());
            return -1;
        }
        if(info.empty())
        {
            fprintf(stderr,"empty format info!\r\n");
            return -1;
        }
        if(!dump_file.empty())
        {
            FILE *fp=fopen(dump_file.c_str(),"w+");
            if(!fp)
            {
                fprintf(stderr,"open %s failed[%s]\r\n",dump_file.c_str(),strerror(platform::getErrno()));
                return -1;
            }
            fwrite(info.c_str(),info.size(),1,fp);
            fclose(fp);
        }
        fprintf(stderr,"%s",info.c_str());
        return 0;
    }
    else {
        return -1;
    }
}

void ChangelogFormatter::printHelpInfo()
{
    static const char helpinfo[]="Usage:\r\n"
                                 "\t tool -f <input_file_name> -o [output_file_name]\r\n"
                                 "\t tool -f <input_file_name> -c <check_version>\r\n"
                                 "\t tool -f <input_file_name> -x <xml/json/html> [-s <dump_version>] [-d <save file name>]\r\n"
                                 "\t tool -h|--help print this page\r\n";
    printf("%s\r\n",helpinfo);
}
