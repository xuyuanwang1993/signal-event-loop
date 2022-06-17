#pragma once

#include <stdint.h>

namespace VDProtocol
{

#pragma pack(push, 1)

struct Frame {
	uint16_t version;
};

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable:4200)
#endif//_MSC_VER

struct FrameV1 {
	Frame frame;
	uint16_t compress_flag;//0:无压缩; >0:对应具体的压缩方案
	uint32_t stream_offset;//二进制流相对于body的偏移量, ~0:没有二进制流; 0:没有json体;
	uint8_t body[0];//json格式, 如果有二进制流需要传输, 则json后面紧接二进制流, 二进制流起始地址参见stream_offset
};

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif//_MSC_VER

struct StreamV1 {
	uint32_t length;//sizeof(Stream) + body length
	FrameV1 frame;
};

#pragma pack(pop)

}//namespace VDProtocol