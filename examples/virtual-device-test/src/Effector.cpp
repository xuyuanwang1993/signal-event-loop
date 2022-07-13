#include "VirtualDevice/Effector.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"
#include <memory>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#	define alloca _malloca
#endif

namespace VirtualDevice
{

void
Effector::GetYuedongVersion()
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::QUERY_YUEDONG_VERSION;
	Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::SetChannelVolume(VolumeChannel channel, uint8_t volume) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::EffectorVolume);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_VOLUME;

	VDProtocol::Caller::EffectorVolume *p_body = (decltype(p_body))p_stream->data;
	p_body->channel = (decltype(p_body->channel))channel;
	p_body->volume = volume;

	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::SetChannelMute(VolumeChannel channel, bool mute) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::EffectorChannelMute);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_CHANNEL_MUTE;

	VDProtocol::Caller::EffectorChannelMute *p_body = (decltype(p_body))p_stream->data;
	p_body->channel = (decltype(p_body->channel))channel;
	p_body->mute = mute;

	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::GetChannelStatus(VolumeChannel channel) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::ChannelVolume);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_CHANNEL_STATUS;

	VDProtocol::ChannelVolume *p_body = (decltype(p_body))p_stream->data;
	*p_body = (decltype(*p_body))channel;

	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::SetSingSwitch(SingSwitch sing_switch) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::SingSwitch);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_SING_SWITCH;

	VDProtocol::Caller::SingSwitch *p_body = (decltype(p_body))p_stream->data;
	p_body->sing_switch = (decltype(p_body->sing_switch))sing_switch;

	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::GetSingSwitch() const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_GET_SING_SWITCH;
	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::SetSoundMode(unsigned sound_mode) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::SoundMode);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::EFFECTOR_SET_SOUND_MODE;

	VDProtocol::Caller::SoundMode *p_body = (decltype(p_body))p_stream->data;
	p_body->sound_mode = (decltype(p_body->sound_mode))sound_mode;

	return Machine::Get()->Write(p_stream, c_stream_length);
}

int
Effector::SetTone(float boost,const std::string &path ) const
{
    do {
        if(path.empty())break;
        std::string data;
        data+="{\"boost\":"+std::to_string(boost)+",\"path\":\""+path+"\"}";
        const size_t c_stream_length = sizeof(VDProtocol::Caller::Body)+ data.size();
        VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
        p_stream->command = VDProtocol::Caller::Command::EFFECTOR_GROUP_SET_TONE;
        memcpy(p_stream->data,data.c_str(),data.size());
        return Machine::Get()->Write(p_stream, c_stream_length);
    } while (0);
    return -1;
}

bool
Effector::DispatchNotify(unsigned command, const void *p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
		case VDProtocol::Caller::Command::EFFECTOR_CHANNEL_STATUS:
		{
			const VDProtocol::Caller::EffectorChannelStatus *p_body = (decltype(p_body))p_notify;
			NotifyChannelStatus((VolumeChannel)p_body->channel, p_body->volume, p_body->mute);
			return true;
		}
		case VDProtocol::Caller::Command::EFFECTOR_GET_SING_SWITCH:
		{
			const VDProtocol::Caller::SingSwitch *p_body = (decltype(p_body))p_notify;
			NotifySingSwitch(p_body->sing_switch);
			return true;
		}
		case VDProtocol::Caller::Command::QUERY_YUEDONG_VERSION:
			NotifyGetYuedongVersion((YuedongVersion*)p_notify);
			return true;
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice
