#include "VirtualDevice/Module5G8.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

bool
Module5G8::Pair() const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::MODULE_5G8_PAIR;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
Module5G8::DispatchNotify(unsigned command, const void* p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
	case VDProtocol::Caller::Command::MODULE_5G8_STATUS:
		NotifyStatus(*(uint8_t*)p_notify);
		return true;
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice