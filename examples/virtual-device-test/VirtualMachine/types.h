#pragma once

namespace VDProtocol
{

enum class ChannelVolume : unsigned {
	HEADSET_MUSIC,
	HEADSET_MIC,
	HEADSET_REVERB,
	BOX_MUSIC,
	BOX_MIC,

	HEADSET_LEFT,
	HEADSET_RIGHT,

	REMOTE,
	RECORD_NET_AUDIO,

	COUNT
};

}//namespace VDProtocol