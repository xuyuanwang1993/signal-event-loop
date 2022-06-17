#include "VirtualDevice/Console.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

bool
Console::SetPanelLight(PanelLight light, uint8_t state) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::ConsolePanelLight);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::CONSOLE_PANEL_LIGHT;

	VDProtocol::Caller::ConsolePanelLight *p_body = (decltype(p_body))p_stream->data;
	p_body->light = (decltype(p_body->light))light;
	p_body->state = state;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
Console::SetLight(LightColor color, LightMode mode, uint8_t brightness) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::ConsoleLight);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::CONSOLE_LIGHT;

	VDProtocol::Caller::ConsoleLight *p_body = (decltype(p_body))p_stream->data;
	p_body->color = (decltype(p_body->color))color;
	p_body->mode = (decltype(p_body->mode))mode;
	p_body->brightness = brightness;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
Console::PowerOff(uint8_t target) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::ConsolePowerOff);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::CONSOLE_POWER_OFF;

	VDProtocol::Caller::ConsolePowerOff *p_body = (decltype(p_body))p_stream->data;
	p_body->target = target;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
Console::DispatchNotify(unsigned command, const void* p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
		case VDProtocol::Caller::Command::CONSOLE_KEY:
		{
			const VDProtocol::Caller::ConsoleKey *p_body = (decltype(p_body))p_notify;
			NotifyKey(p_body->key_code, p_body->state);
			return true;
		}
		case VDProtocol::Caller::Command::CONSOLE_VOLUME:
		{
			const VDProtocol::Caller::ConsoleVolume *p_body = (decltype(p_body))p_notify;
			NotifyVolume((VolumeChannel)p_body->channel, p_body->volume);
			return true;
		}
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice