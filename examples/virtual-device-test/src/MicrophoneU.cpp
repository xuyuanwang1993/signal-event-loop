#include "VirtualDevice/MicrophoneU.h"
#include "VirtualMachine/protocol_caller.h"
#include "VirtualDevice/Machine.h"

namespace VirtualDevice
{

bool
MicrophoneU::GetModuleChannel(uint8_t device_id) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(uint8_t);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_CHANNEL;

	uint8_t *p_device_id = (decltype(p_device_id))p_stream->data;
	*p_device_id = device_id;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
MicrophoneU::GetModuleSignalStrength(uint8_t device_id) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(uint8_t);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_SIGNAL_STRENGTH;

	uint8_t *p_device_id = (decltype(p_device_id))p_stream->data;
	*p_device_id = device_id;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
MicrophoneU::PairMic(uint8_t device_id, bool left, bool right) const
{
	size_t stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::MicUPair);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_PAIR;

	VDProtocol::Caller::MicUPair *p_body = (decltype(p_body))p_stream->data;
	p_body->device_id = device_id;
	p_body->left = left;
	p_body->right = right;

	return Machine::Get()->Write(p_stream, stream_length) == 0;
}

bool
MicrophoneU::ScanFreqBand(uint8_t device_id, bool is_left, unsigned freq_channel_begin, unsigned freq_channel_end) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::MicUScanFreqBand);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_SCAN_FREQ_BAND;

	VDProtocol::Caller::MicUScanFreqBand *p_body = (decltype(p_body))p_stream->data;
	p_body->device_id = device_id;
	p_body->is_left = is_left;
	p_body->freq_channel_begin = freq_channel_begin;
	p_body->freq_channel_end = freq_channel_end;

	return Machine::Get()->Write(p_stream, c_stream_length) == 0;
}

bool
MicrophoneU::GetFreeFreqBand(uint8_t device_id, unsigned channel, unsigned freq_channel_begin, unsigned freq_channel_end, unsigned threshold_rssi, unsigned threshold_snr) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::MicUGetFreeFreqBand);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_GET_FREE_FREQ_BAND;

	VDProtocol::Caller::MicUGetFreeFreqBand *p_body = (decltype(p_body))p_stream->data;
	p_body->device_id = device_id;
	p_body->channel = channel;
	p_body->freq_channel_begin = freq_channel_begin;
	p_body->freq_channel_end = freq_channel_end;
	p_body->threshold_rssi = threshold_rssi;
	p_body->threshold_snr = threshold_snr;

	return Machine::Get()->Write(p_stream, c_stream_length) == 0;
}

bool
MicrophoneU::SetFreqBand(uint8_t device_id, bool is_left, unsigned freq_band) const
{
	const size_t c_stream_length = sizeof(VDProtocol::Caller::Body) + sizeof(VDProtocol::Caller::MicUSetFreqBand);
	VDProtocol::Caller::Body *p_stream = (decltype(p_stream))alloca(c_stream_length);
	p_stream->command = VDProtocol::Caller::Command::MIC_U_SET_FREQ_BAND;

	VDProtocol::Caller::MicUSetFreqBand *p_body = (decltype(p_body))p_stream->data;
	p_body->device_id = device_id;
	p_body->is_left = is_left;
	p_body->freq_band = freq_band;

	return Machine::Get()->Write(p_stream, c_stream_length) == 0;
}

bool
MicrophoneU::DispatchNotify(unsigned command, const void* p_notify)
{
#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wswitch"
#endif//__GNUC__

	switch ((VDProtocol::Caller::Command)command) {
	case VDProtocol::Caller::Command::MIC_U_CHANNEL:
	{
		const VDProtocol::Caller::MicUModuleChannel *p_body = (decltype(p_body))p_notify;
		ChannelState state;
		state.left_channel = p_body->left_channel;
		state.right_channel = p_body->right_channel;
		state.left_connect = p_body->left_connect;
		state.right_connect = p_body->right_connect;
		NotifyGetModuleChannel(p_body->device_id, state);
		return true;
	}
	case VDProtocol::Caller::Command::MIC_U_SIGNAL_STRENGTH:
	{
		const VDProtocol::Caller::MicUModuleSignalStrength *p_body = (decltype(p_body))p_notify;
		NotifyGetModuleSignalStrength(p_body->device_id, p_body->left_rssi, p_body->right_rssi);
		return true;
	}
	case VDProtocol::Caller::Command::MIC_U_SCAN_FREQ_BAND:
	{
		const VDProtocol::Caller::MicUReplyScanFreqBand *p_body = (decltype(p_body))p_notify;
		NotifyScanChannel(p_body->device_id, p_body->is_left, (ChannelInfo*)p_body->info, p_body->count_info);
		return true;
	}
	case VDProtocol::Caller::Command::MIC_U_GET_FREE_FREQ_BAND:
	{
		const VDProtocol::Caller::MicUReplyGetFreeFreqBand *p_body = (decltype(p_body))p_notify;
		NotifyGetFreeBand(p_body->device_id, p_body->is_left, p_body->band, p_body->rssi, p_body->snr);
		return true;
	}
	case VDProtocol::Caller::Command::MIC_U_SET_FREQ_BAND:
	{
		const VDProtocol::Caller::MicUReplySetFreqBand *p_body = (decltype(p_body))p_notify;
		NotifySetFreqBand(p_body->device_id, p_body->is_left, p_body->band);
		return true;
	}
	}

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif//__GNUC__

	return false;
}

}//namespace VirtualDevice