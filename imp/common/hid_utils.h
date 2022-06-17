#ifndef HID_UTILS_H
#define HID_UTILS_H
#include "log/aimy-log.h"
#include<list>
namespace aimy {

enum HidStatus:uint8_t{
    HID_OK,//read/write success try again ,
    HID_ERROR,// maybe something error? need reopen device
};

class HidDevice{
public:
    static int open(const std::string &_path);
    static void close(int hid);

    static std::pair<HidStatus,ssize_t>hidRead(int hid,void *buf,size_t max_len);
    static std::pair<HidStatus,ssize_t>hidWrite(int hid,void *buf,size_t data_len);

    static std::list<std::string> hidFind(uint16_t product_id,uint16_t vendor_id);
};
}
#endif // HID_UTILS_H
