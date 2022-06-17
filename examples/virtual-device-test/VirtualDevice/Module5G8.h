#pragma once

#include <stdint.h>

namespace VirtualDevice
{

class Module5G8
{
public:
	/*
	功能说明: 配对
	*/
	bool Pair() const;

protected:
    virtual void NotifyStatus(uint8_t status) {}

protected:
	bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice