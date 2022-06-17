#pragma once

#include <stdint.h>
#include <string>
#include "types.h"

namespace VirtualDevice
{

class Effector
{
public:
	void GetYuedongVersion();
	/*
	 * 当没有控台时才调用, 有控台时, virtual_device会自行设置音量, 并回调Console::NotifyVolume
	 * volume: [0, 100]
	 */
	int SetChannelVolume(VolumeChannel channel, uint8_t volume) const;
	/*
	 * 静音
	 */
	int SetChannelMute(VolumeChannel channel, bool mute) const;
	/*
	 * 读取通道状态, 音量、静音
	 */
	int GetChannelStatus(VolumeChannel channel) const;
	/*
	* 原/伴唱选择
	*/
	int SetSingSwitch(SingSwitch sing_switch) const;
	int GetSingSwitch() const;

	/*
	* sound_mode: 0: 专业; 1: KTV; 2: 清唱; 3: 戏曲; 4: 朗诵; 5: 活力; 6: 高亢; 7: 低沉; 8: 明亮;
	*/
	int SetSoundMode(unsigned sound_mode) const;

	enum class Pitch {
		LOW,
		MID,
		HIGH,
	};
	/*
	* 设置/读取音调
	*/
	int SetTone(float boost,const std::string &path ) const;
	enum class RecordType {
		MIXING,
		WET,
	};
	/*
	* 设置/读取音色
	*/
	int SetTimbre(RecordType type, unsigned level);
	int GetTimbre(RecordType type);
	/*
	* 设置/读取气息
	*/
	int SetEnergy(RecordType type, unsigned level);
	int GetEnergy(RecordType type);

	virtual void NotifyChannelStatus(VolumeChannel channel, uint8_t volume, bool mute) {}
	virtual void NotifySingSwitch(uint8_t sing_switch) {}
	virtual void NotifyGetTone(float boost) {}
	virtual void NotifyGetTimbre(unsigned level) {}
	virtual void NotifyGetEnergy(unsigned level) {}

#pragma pack(push, 1)
	struct YuedongVersion {
		enum {
			SCM,
			FPGA,
			ARM,
			DSP,
		} type;
		char version[0x20];
	};
#pragma pack(pop)
	virtual void NotifyGetYuedongVersion(const YuedongVersion *cp_dev_info) {}

protected:
	bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice
