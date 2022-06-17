#ifndef PROTOCALNORMAL_H
#define PROTOCALNORMAL_H
#include "base/protocal-base.h"
namespace aimy {
class ProtocalNormal :public ProtocalBase{
public:
    ProtocalNormal(uint32_t _max_cache_size=32*1024,uint32_t _max_frame_size=4096,uint32_t _slice_size=4096);
    ~ProtocalNormal() override;
};
}
#endif // PROTOCALNORMAL_H
