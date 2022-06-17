#pragma once

#include <stdint.h>
#include "types.h"

namespace VirtualDevice
{

class Console
{
public:
	enum class PanelLight : uint8_t
	{
		WIRED,			//有线网络
		WIRELESS,		//无线网络
		MOBILE,			//4G网络
		ACCESS_POINT,	//热点
		BLUETOOTH,		//蓝牙
		MUTE,			//静音
	};
	/*
	功能说明: 设置面板灯
	state:	有线网络、4G网络: 0: 无传输信号; 1: 有效传输; 2: 网络不稳定;
			无线网络、热点、蓝牙、静音: 0: 关闭状态; 1: 打开状态;
	*/
	bool SetPanelLight(PanelLight light, uint8_t state) const;

	/*
	* 功能说明: 灯光控制;
	* LightColor: (R << 16) | (G << 8) | B
	* brightness: [0, 100]
	*/
	typedef uint32_t LightColor;
	enum class LightMode : uint8_t {
		ALWAYS,			//常亮
		GRADIENT,		//渐变
		MUSIC_CONTROL,	//音乐控制
	};
	bool SetLight(LightColor color, LightMode mode, uint8_t brightness) const;

	/*
	功能说明: 关机
	target: 1: 12V电源;
	*/
	bool PowerOff(uint8_t target) const;

	/*
	 * 按键
	 * key_code: 0: 功能按键;
				1: 搜索按键;
				2: 录音按键;
				3: 播放/暂停按键;
				4: 重唱按键;
				5: 声道按键;
				6: 切歌按键;
				7: 原唱按键;
				8: 伴唱按键;
				9: 后台按键;
	 * state:	0: 不支持;
				1: 按下;
				2: 松开;
	 */
	virtual void NotifyKey(unsigned key_code, uint8_t state) const {}

	/*
	 * volume: [0, 100]
	 */
	virtual void NotifyVolume(VolumeChannel channel, uint8_t volume) const {}

protected:
	virtual bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice