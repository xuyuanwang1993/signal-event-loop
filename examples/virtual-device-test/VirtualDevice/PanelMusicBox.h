#pragma once

#include <stdint.h>
#include "types.h"

namespace VirtualDevice
{

class PanelMusicBox
{
public:
	enum class Light : uint8_t {
		ANALOG_MIC_IN_1,
		ANALOG_MIC_IN_2,
		DIGITAL_IN_1,
		DIGITAL_IN_2,
		OPTICAL_IN,
		DIGITAL_OUT_1,
		DIGITAL_OUT_2,
		MUTE,
		WIRED,
		WIRELESS,
		MOBILE,
		ACCESS_POINT,
		BLUETOOTH,
		DT_NETWORK,
		INFRARED_RECEIVER,
		POWER,
	};
	enum class State : uint8_t {
		OFF,	//熄灭
		ON,		//常亮
		FLICKER,//闪烁
	};
	/**
	 * @param brightness [0, 10]
	 */
	bool SetLight(Light light, State state, uint8_t brightness) const;

	enum class Key : uint8_t {
		POWER,
	};
	enum class Action : uint8_t {
		LOOSEN,		//松开
		PRESSING,	//按下
	};
	virtual void NotifyKey(Key key, Action action) const {}

protected:
	virtual bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice