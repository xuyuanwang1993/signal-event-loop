#pragma once

namespace VirtualDevice
{

enum class VolumeChannel : unsigned {
	HEADSET_MUSIC,
	HEADSET_MIC,
	HEADSET_EFFECT,
	BOX_MUSIC,
	BOX_MIC,

	HEADSET_LEFT,
	HEADSET_RIGHT,

	GUIDE_SING,
	CHORUS,
};

enum class SingSwitch : uint8_t {
	ACCOMPANIMENT,	//伴唱, 对于E5+音箱，指定使用主板音频信号
	SHAKELIGHT,		//原唱, 对于E5+音箱，指定使用蓝牙音频信号
};

}//namespace VirtualDevice