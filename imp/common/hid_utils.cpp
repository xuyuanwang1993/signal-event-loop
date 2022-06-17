#include "hid_utils.h"
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include <ctype.h>
#include <dirent.h>
using namespace aimy;

int HidDevice::open(const std::string &_path)
{
    auto hid=::open(_path.c_str(),O_RDWR|O_CLOEXEC);
    if(hid<=0)
    {
        AIMY_ERROR("open hid[%s] failed[%s]",_path.c_str(),strerror(errno));
    }
    return hid>0;
}

void HidDevice::close(int hid)
{
    if(hid>0)::close(hid);
}

std::pair<HidStatus,ssize_t> HidDevice::hidRead(int hid,void *buf,size_t max_len)
{
    HidStatus status=HID_ERROR;
    ssize_t result=-1;
    if(hid<0)
    {
        AIMY_ERROR("hidRead failed [not open device]");
        return {status,result};
    }
    status=HID_OK;
    result=::read(hid,buf,max_len);
    if(result<0)
    {
        int err=errno;
        switch (err) {
        case EAGAIN:
        case EINTR:
            break;
        default:
            status=HID_ERROR;
        }
        AIMY_ERROR("hidRead failed [%s]",strerror(err));
    }
    return {status,result};
}

std::pair<HidStatus,ssize_t> HidDevice::hidWrite(int hid,void *buf,size_t data_len)
{
    HidStatus status=HID_ERROR;
    ssize_t result=-1;
    if(hid<0)
    {
        AIMY_ERROR("hidWrite failed [not open device]");
        return {status,result};
    }
    status=HID_OK;
    result=::write(hid,buf,data_len);
    if(result<0)
    {
        int err=errno;
        switch (err) {
        case EAGAIN:
        case EINTR:
            break;
        default:
            status=HID_ERROR;
        }
        AIMY_ERROR("hidWrite failed [%s]",strerror(err));
    }
    return {status,result};
}

std::list<std::string> HidDevice::hidFind(uint16_t product_id, uint16_t vendor_id)
{
    std::list<std::string> result;

    static const char* const hid_device_prefix= "/sys/bus/hid/devices/";
    static const char* const dev_path_prefix = "/dev/";
    do{
        DIR* devices = opendir(hid_device_prefix);
        if (!devices) {
            AIMY_ERROR("opendir hid file system failed:[%s]",strerror(errno));
            break;
        }
        struct dirent* entry;
        uint16_t vid, pid;
        uint16_t tmp1, tmp2;


        while ((entry = readdir(devices))) {
            if(entry->d_type!=DT_LNK)continue;
            AIMY_DEBUG("dev:%s",entry->d_name);
            if (sscanf(entry->d_name, "%04hx:%04hx:%04hx.%04hx", &tmp1, &vid, &pid, &tmp2) != 4)
                continue;
            if (vid != vendor_id || pid != product_id)
                continue;
            std::string hid_path=hid_device_prefix;
            hid_path+=entry->d_name+std::string("/hidraw");

            DIR* hidraw_dir = opendir(hid_path.c_str());
            if (!hidraw_dir) {
                AIMY_ERROR("opendir hidraw failed:[%s]",strerror(errno));
                continue;
            }
            struct dirent* hid_entry;
            while ((hid_entry = readdir(hidraw_dir))) {
                if(hid_entry->d_type!=DT_DIR)continue;
                if (sscanf(hid_entry->d_name, "hidraw%hd", &tmp1) != 1)
                    continue;
                result.push_back(dev_path_prefix+std::string(hid_entry->d_name));
                break;
            }
            closedir(hidraw_dir);
        }
        //release
        if(devices)
        {
            closedir(devices);
        }
    }while(0);
    return result;
}
