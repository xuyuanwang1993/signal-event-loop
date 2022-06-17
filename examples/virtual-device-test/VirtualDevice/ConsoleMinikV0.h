#pragma once

#include "Console.h"

namespace VirtualDevice
{

class ConsoleMinikV0 : public Console
{
public:
	enum class GameStatus : uint8_t {
		STANDBY,	//待机
		NORMAL,		//正常工作
		FEE_SING,	//收费唱
		FREE_SING,	//免费唱
		FREE_LISTEN,//免费听
		RECORDING,	//正在录音
		PLAYBACK,	//录音回放
		GAMEOVER,	//游戏结束
		AGING_TEST,	//老化测试
		REBOOT,		//系统重启
		POWER_OFF,	//系统断电
		WELCOME,	//欢迎登台状态
		SINGING,	//演唱状态
		SINGING_END,//演唱结束
		WAITING,	//换人等待
		SONG_LOADING,//歌曲加载
		IN_LOTTERY,	//抽奖中
	};
	void SetGameStatus(GameStatus status);
	/*
	* 设置电平
	* [0, 15]
	*/
	void SetLevel(uint8_t dry, uint8_t music, uint8_t headphone, uint8_t record);

	virtual void NotifyInsertCoin() const {}//投币
	/*
	* 话筒状态
	* mic_id: 1:话筒1(右话筒);	2:话筒2(左话筒);
	* state: 1:被使用;	0:未被使用;
	*/
	virtual void NotifyMicState(unsigned mic_id, unsigned state) const {}
	/*
	* 控台状态
	* status: 1:本机工作正常;		0:本机工作不正常;
	*/
	virtual void NotifyStatus(unsigned status) const {}

protected:
	virtual bool DispatchNotify(unsigned command, const void *p_notify) override;
};

}//namespace VirtualDevice