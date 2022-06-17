#include "VirtualDevice/ConsoleMinikV0.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

void
ConsoleMinikV0::SetGameStatus(GameStatus status)
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::ConsoleGameStatus);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::CONSOLE_GAME_STATUS;

	VDProtocol::Caller::ConsoleGameStatus *p_body = (decltype(p_body))p_stream->data;
	p_body->status = (uint8_t)status;

	Machine::Get()->Write(p_stream, stream_length);
}

void
ConsoleMinikV0::SetLevel(uint8_t dry, uint8_t music, uint8_t headphone, uint8_t record)
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::ConsoleLevel);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::CONSOLE_LEVEL;

	VDProtocol::Caller::ConsoleLevel *p_body = (decltype(p_body))p_stream->data;
	p_body->dry = dry;
	p_body->music = music;
	p_body->headphone = headphone;
	p_body->record = record;

	Machine::Get()->Write(p_stream, stream_length);
}

bool
ConsoleMinikV0::DispatchNotify(unsigned command, const void *p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
		case VDProtocol::Caller::Command::CONSOLE_MIC_STATE:
		{
			const VDProtocol::Caller::ConsoleMicState *p_body = (decltype(p_body))p_notify;
			NotifyMicState(p_body->mic_id, p_body->state);
			return true;
		}
		case VDProtocol::Caller::Command::CONSOLE_STATUS:
		{
			const VDProtocol::Caller::ConsoleStatus *p_body = (decltype(p_body))p_notify;
			NotifyStatus(p_body->status);
			return true;
		}
		case VDProtocol::Caller::Command::CONSOLE_INSERT_COIN:
			NotifyInsertCoin();
			return true;
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return Console::DispatchNotify(command, p_notify);
}

}//namespace VirtualDevice