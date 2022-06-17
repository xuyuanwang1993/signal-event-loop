#include "VirtualDevice/PanelMusicBox.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

bool
PanelMusicBox::SetLight(Light light, State state, uint8_t brightness) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::MusicBoxPanelLight);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::MUSIC_BOX_PANEL_LIGHT;

	VDProtocol::Caller::MusicBoxPanelLight *p_body = (decltype(p_body))p_stream->data;
	p_body->light = (decltype(p_body->light))light;
	p_body->state = (decltype(p_body->state))state;
	p_body->brightness = brightness;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
PanelMusicBox::DispatchNotify(unsigned command, const void* p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
    case VDProtocol::Caller::Command::MUSIC_BOX_PANEL_KEY:
    {
		const VDProtocol::Caller::MusicBoxPanelKey *p_body = (decltype(p_body))p_notify;
        NotifyKey((Key)p_body->key, (Action)p_body->action);
        return true;
    }
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice