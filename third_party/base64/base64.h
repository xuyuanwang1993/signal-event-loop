#ifndef BASE64_H
#define BASE64_H
#include <memory>

namespace aimy {
std::pair<std::shared_ptr<uint8_t>, uint32_t> base64Encode(const void *input_buf, uint32_t buf_size);
std::pair<std::shared_ptr<uint8_t>, uint32_t> base64Decode(const void *input_buf, uint32_t buf_size);
}
#endif // BASE64_H
