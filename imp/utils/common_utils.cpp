#include "common_utils.h"
#include<string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/file.h>
#include "log/aimy-log.h"
bool AIMY_UTILS::acquireSigleInstanceLcok(const std::string &lock_file_path)
{
    //
    bool ret=false;
    int fd=-1;
    do{
        auto process_pid=getpid();
        auto process_name=readProcName(process_pid);
        if(process_name.empty())
        {
            AIMY_ERROR("get pid[%d] process name failed [%s]",process_pid,strerror(errno));
            break;
        }
        std::string lockFileName=lock_file_path+"/"+process_name+".pid";

        fd=open(lockFileName.c_str(),O_RDWR|O_CREAT|O_CLOEXEC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if(fd<=0)
        {
            AIMY_ERROR("open %s failed[%s]",lockFileName.c_str(),strerror(errno));
            break;
        }
        AIMY_INFO("open %s success",lockFileName.c_str());
        int flags=LOCK_EX;;
        int iret=0;
        iret = flock(fd,flags);
        if(iret!=0)
        {
            AIMY_ERROR("%s is locking[%d->%s]",lockFileName.c_str(),iret,strerror(errno));
            break;
        }
        AIMY_INFO("flock %s success",lockFileName.c_str());
        //check pid
        char buf[32]={0};
        memset(buf,0,32);
        auto len=read(fd,buf,31);
        bool isexisted=false;
        if(len>0)
        {
            auto work_pid=std::stoi(buf);
            if(work_pid!=process_pid)
            {
                auto work_process_name=readProcName(work_pid);
                if(work_process_name==process_name)
                {
                    isexisted=true;
                }
            }
        }
        if(isexisted)
        {
            AIMY_ERROR("process %s is working!",process_name.c_str());
            break;
        }
        //write pid
        FILE *fp=fopen(lockFileName.c_str(),"w+");
        if(!fp)
        {
            AIMY_ERROR("open %s error[%s]!",lockFileName.c_str(),strerror(errno));
            break;
        }
        fprintf(fp,"%d",process_pid);
        fsync(fileno(fp));
        fclose(fp);
        ret=true;
    }while(0);
    if(fd>0)
    {
        flock(fd,LOCK_UN);
        ::close(fd);
    }
    return ret;
}

std::string AIMY_UTILS::readProcName(pid_t pid)
{
    std::string ret;
    char buf[32]={0};
    sprintf(buf,"/proc/%d/stat",pid);
    FILE *fp=fopen(buf,"r");
    do{
        if(!fp)break;
        constexpr uint32_t max_read_size=512;
        char read_buf[max_read_size]={0};
        memset(read_buf,0,max_read_size);
        auto read_len=fread(read_buf,1,max_read_size,fp);
        if(read_len<=1)break;
        int t_pid=0;
        char pro_name[256];
        sscanf(read_buf,"%d (%[^)]",&t_pid,pro_name);
        ret=pro_name;
    }while(0);
    if(fp)fclose(fp);
    return ret;
}

uint8_t AIMY_UTILS::genCheckCode(const void *p_data, uint32_t length)
{
    const uint8_t *p_src = (decltype(p_src))p_data;
    const uint64_t c_unit_count = length / sizeof(*p_src);
    uint8_t check_code = 0;
    for (uint64_t i = 0; i < c_unit_count; ++i)
        check_code ^= p_src[i];
    return check_code;
}

bool AIMY_UTILS::checkIsBuildDay()
{
    auto ptr=getenv("DAY_DEBUG");
    if(ptr&&std::stoi(ptr)!=0)
    {
        std::string build_time=__DATE__;
        auto now = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(now);
        char buf[512]={0};
        struct tm tmp;
        strftime(buf,512,"%b %d %Y",localtime_r(&tt,&tmp));
        return build_time==buf;
    }
    else {
        return false;
    }
}

std::list<std::list<std::string>> AIMY_UTILS::parserCommandConfigFile(const std::string &file_name)
{
    std::list<std::list<std::string>> result;
    auto fp=fopen(file_name.c_str(),"r");
    if(!fp){
        AIMY_ERROR("open %s failed![%s]",file_name.c_str(),strerror(errno));
        return result;
    }
    char *line=nullptr;
    size_t len=0;
    ssize_t read_len=-1;
    auto check_func=[](char c)->bool
    {
        return c!=' '&&c!='\r'&&c!='\n'&&c!='\t';
    };
    while(1)
    {
        read_len=getline(&line,&len,fp);
        if(read_len<=0)break;
        //parse
        std::list<std::string> line_str_list;
        for(ssize_t i=0;i<read_len;++i)
        {
            if(!check_func(line[i]))continue;
            ssize_t j=i+1;
            while(j<read_len)
            {
                if(!check_func(line[j]))break;
                ++j;
            }
            line_str_list.push_back(std::string(line+i,j-i));
            i=j;
        }
        if(!line_str_list.empty())result.push_back(std::move(line_str_list));
    }
    fclose(fp);
    return result;
}
