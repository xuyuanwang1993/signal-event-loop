#ifndef IUTILS_H
#define IUTILS_H
#include<stdint.h>
#include<string>
#include<memory>
namespace aimy {
class Iutils {
public:
    static uint16_t crc_rc16(const void * data,uint32_t data_len);
    static void sha256(const void *data,uint32_t data_len,uint8_t out[32]);
    static std::string getSuffix(const std::string &input,const std::string &separator=".");
    /**
     * @brief allocBuffer
     * @param len
     * @return
     * return buffer will be filled with zero
     */
    static std::shared_ptr<uint8_t> allocBuffer(uint32_t len);
};
}
#endif // IUTILS_H
