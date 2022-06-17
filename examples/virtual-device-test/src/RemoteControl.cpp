#include "VirtualDevice/RemoteControl.h"
#include "VirtualMachine/protocol_caller.h"

namespace VirtualDevice
{

bool
RemoteControl::DispatchNotify(unsigned command, const void *p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
		case VDProtocol::Caller::Command::REMOTE_KEY:
		{
			const VDProtocol::Caller::RemoteKey *p_body = (decltype(p_body))p_notify;
			NotifyKey((Action)p_body->action, p_body->key_code, p_body->user_code);
			return true;
		}
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice