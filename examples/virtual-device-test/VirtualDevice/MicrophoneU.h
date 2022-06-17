#pragma once

#include <stdint.h>

namespace VirtualDevice
{

class MicrophoneU
{
public:
	/*
	功能说明: 获取模块的通道
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	*/
	bool GetModuleChannel(uint8_t device_id) const;

	/*
	功能说明: 获取模块的信号强度
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	*/
	bool GetModuleSignalStrength(uint8_t device_id) const;

	/*
	功能说明: 配对
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	left, right: 要配对左/右话筒, 对应的置true
	*/
	bool PairMic(uint8_t device_id, bool left, bool right) const;

	/*
	功能说明: 扫描可获取到的频段
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	freq_channel_begin, freq_channel_end: [1, 150], 一次扫描的频道控制在50以内, 避免等待时间过长
	*/
	bool ScanFreqBand(uint8_t device_id, bool is_left, unsigned freq_channel_begin, unsigned freq_channel_end) const;

	/*
	功能说明: 获取空闲频段, 返回第一个符合频道需求的空闲频道
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	channel: 0: 左声道; 1: 右声道; 2: all;
	freq_channel_begin, freq_channel_end: [1, 150], 一次扫描的频道控制在50以内, 避免等待时间过长
	threshold_rssi: [12, ++], 推荐0x0C
	threshold_snr: [20, ++], 推荐0x26
	*/
	bool GetFreeFreqBand(uint8_t device_id, unsigned channel, unsigned freq_channel_begin, unsigned freq_channel_end, unsigned threshold_rssi, unsigned threshold_snr) const;

	/*
	功能说明: 设定频道
	device_id: 0xF1  0xF2  0xF3(目前只有这3个，F1是外置的TYPE C1 F2是TYPE C2 F3是板载的，0xff作为广播)
	*/
	bool SetFreqBand(uint8_t device_id, bool is_left, unsigned freq_band) const;

	struct ChannelState {
		unsigned left_channel;
		unsigned right_channel;
		bool left_connect;
		bool right_connect;
	};
	virtual void NotifyGetModuleChannel(uint8_t device_id, const ChannelState &state) {}
	virtual void NotifyGetModuleSignalStrength(uint8_t device_id, uint8_t left_rssi, uint8_t right_rssi) {}
#pragma pack (push, 1)
	struct ChannelInfo {
		uint8_t rssi;
		uint8_t snr;
	};
#pragma pack(pop)
	virtual void NotifyScanChannel(uint8_t device_id, bool is_left, const ChannelInfo *p_info, unsigned count_info) {}
	virtual void NotifyGetFreeBand(uint8_t device_id, bool is_left, uint8_t band, uint8_t rssi, uint8_t snr) {}
	virtual void NotifySetFreqBand(uint8_t device_id, bool is_left, uint8_t band) {}

protected:
	bool DispatchNotify(unsigned command, const void *p_notify);
};

}//namespace VirtualDevice