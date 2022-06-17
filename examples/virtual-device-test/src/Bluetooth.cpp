#include "VirtualDevice/Bluetooth.h"
#include <string.h>
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

const char* const Bluetooth::scm_cmd_power_on = "AT#MW\r\n";
const char* const Bluetooth::scm_cmd_query_name = "AT#MM\r\n";
const char* const Bluetooth::scm_cmd_conference = "AT#CT\r\n";
const char* const Bluetooth::scm_cmd_pair = "AT#CA\r\n";
const char* const Bluetooth::scm_cmd_pair_cancel = "AT#CB\r\n";
const char* const Bluetooth::scm_cmd_pair_accept = "AT#QJ\r\n";
const char* const Bluetooth::scm_cmd_device_status = "AT#CY\r\n";

bool
Bluetooth::PowerOn() const
{
	return BluetoothCommand(scm_cmd_power_on);
}

bool
Bluetooth::QueryName() const
{
	return BluetoothCommand(scm_cmd_query_name);
}

bool
Bluetooth::Conference() const
{
	return BluetoothCommand(scm_cmd_conference);
}

bool
Bluetooth::Pair() const
{
	return BluetoothCommand(scm_cmd_pair);
}

bool
Bluetooth::PairCancel() const
{
	return BluetoothCommand(scm_cmd_pair_cancel);
}

bool
Bluetooth::PairAccept() const
{
	return BluetoothCommand(scm_cmd_pair_accept);
}

bool
Bluetooth::GetDeviceStatus() const
{
	return BluetoothCommand(scm_cmd_device_status);
}

bool
Bluetooth::BluetoothCommand(const char *str_command) const
{
	const uint8_t c_len_command = strlen(str_command) + 1;
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + c_len_command;
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::BLUE_TOOTH_COMMAND;
	memcpy(p_stream->data, str_command, c_len_command);
	return Machine::Get()->Write(p_stream, c_stream_length) == 0;
}

bool
Bluetooth::DispatchNotify(unsigned command, const void *p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
	case VDProtocol::Caller::Command::BLUE_REPLY:
	{
		const VDProtocol::Caller::BlueReply *p_body = (decltype(p_body))p_notify;
		NotifyReply(p_body->msg);
		return true;
	}
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice