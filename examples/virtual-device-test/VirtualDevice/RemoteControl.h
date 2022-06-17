#pragma once

#include <stdint.h>

namespace VirtualDevice
{

class RemoteControl
{
public:
	enum class Action : uint8_t {
		PRESS = 0,				//首次按下
		KEEP_PRESSING = 0xFF	//一直按下
	};
	virtual void NotifyKey(Action action, uint8_t key_code, uint16_t user_code) {}

protected:
	virtual bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice