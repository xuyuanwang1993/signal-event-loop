#pragma once

#include <stdint.h>
#include "types.h"

namespace VDProtocol
{
namespace Caller
{

enum class Command : uint16_t
{
	UNKNOWN = 0xFFFF,
    ECHO_COMMAND=UNKNOWN-1,
	QUERY_DEVICE = 0,
	QUERY_YUEDONG_VERSION,
	QUERY_VIRTUAL_DEVICE_VERSION,

	/************ 控台 begin *************/
	CONSOLE_KEY = 100,
	CONSOLE_VOLUME,

	CONSOLE_POWER_OFF,
	CONSOLE_LIGHT,
	CONSOLE_PANEL_LIGHT,

	CONSOLE_MIC_STATE,
	CONSOLE_STATUS,
	CONSOLE_GAME_STATUS,
	CONSOLE_LEVEL,
	CONSOLE_INSERT_COIN,
	/************ 控台 end *************/

	/************ 音乐盒子前面板 begin *************/
	MUSIC_BOX_PANEL_LIGHT = 200,
	MUSIC_BOX_PANEL_KEY,
	/************ 音乐盒子前面板 end *************/

	/************ 遥控器 begin *************/
	REMOTE_KEY = 500,
	/************ 遥控器 end *************/

	/************ 效果器 begin *************/
	EFFECTOR_VOLUME = 550,
	EFFECTOR_SING_SWITCH,
	EFFECTOR_CHANNEL_MUTE,
	EFFECTOR_CHANNEL_STATUS,
	EFFECTOR_GET_SING_SWITCH,
	EFFECTOR_SET_SOUND_MODE,
	EFFECTOR_GROUP_SET_TONE,//设置音调
	/************ 效果器 end *************/

	/************ U段话筒 begin *************/
	MIC_U_CHANNEL = 2000,
	MIC_U_SIGNAL_STRENGTH,
	MIC_U_PAIR,
	MIC_U_SCAN_FREQ_BAND,
	MIC_U_GET_FREE_FREQ_BAND,
	MIC_U_SET_FREQ_BAND,
	/************ U段话筒 end *************/

	/************ 蓝牙 begin *************/
	BLUE_TOOTH_COMMAND = 2100,
	BLUE_REPLY,
	/************ 蓝牙 end *************/

	/************ 5.8G发射模块 begin *************/
	MODULE_5G8_PAIR = 2150,
	MODULE_5G8_STATUS,
	/************ 5.8G发射模块 end *************/

	CLIENT_STARTUP = 3000,
};

#pragma pack(push, 1)

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable:4200)
#endif//_MSC_VER

struct Body {
	Command command;
	uint8_t data[0];
};

struct QueryDevice {
	enum {
		SUCCESS = 0,
		REPLY_FORMAT,			//设备应答解析错误
	} status;
	char product_id[0x20];
	char version[0x20];
	char date[0x20];
};

struct GetYuedongVersion {
	enum {
		SCM,
		FPGA,
		ARM,
		DSP,
	} type;
	char version[0x20];
};

struct ConsoleKey {
	unsigned key_code;
	uint8_t state;
};

struct ConsoleVolume {
	ChannelVolume channel;
	uint8_t volume;
};

struct EffectorChannelMute {
	ChannelVolume channel;
	bool mute;
};

struct EffectorChannelStatus {
	ChannelVolume channel;
	uint8_t volume;
	bool mute;
};

enum class PanelLight : uint8_t
{
	WIRED,			//有线网络
	WIRELESS,		//无线网络
	MOBILE,			//4G网络
	ACCESS_POINT,	//热点
	BLUETOOTH,		//蓝牙
	MUTE,			//静音
};

struct ConsolePanelLight {
	PanelLight light;
	uint8_t state;
};

struct ConsoleLight {
	uint32_t color;
	uint8_t mode;
	uint8_t brightness;
};

struct ConsolePowerOff {
	uint8_t target;
};

struct ConsoleMicState {
	unsigned mic_id;
	unsigned state;
};

struct ConsoleLevel {
	uint8_t dry;
	uint8_t music;
	uint8_t headphone;
	uint8_t record;
};

struct ConsoleStatus {
	unsigned status;
};

struct ConsoleGameStatus {
	uint8_t status;
};

struct MusicBoxPanelKey {
	uint8_t key;
	uint8_t action;
};

struct MusicBoxPanelLight {
	uint8_t light;
	uint8_t state;
	uint8_t brightness;
};

struct RemoteKey {
	uint8_t action;
	uint8_t key_code;
	uint16_t user_code;
};

typedef ConsoleVolume EffectorVolume;

struct SingSwitch {
	uint8_t sing_switch;
};

struct SoundMode {
	uint8_t sound_mode;
};

struct MicUModuleChannel {
	uint8_t device_id;
	unsigned left_channel;
	unsigned right_channel;
	bool left_connect;
	bool right_connect;
};

struct MicUModuleSignalStrength {
	uint8_t device_id;
	uint8_t left_rssi;
	uint8_t right_rssi;
};

struct MicUPair {
	uint8_t device_id;
	bool left;
	bool right;
};

struct MicUScanFreqBand {
	uint8_t device_id;
	bool is_left;
	unsigned freq_channel_begin;
	unsigned freq_channel_end;
};

struct MicUReplyScanFreqBand {
	uint8_t device_id;
	bool is_left;
	unsigned count_info;
	struct {
		uint8_t rssi;
		uint8_t snr;
	} info[0];
};

struct MicUGetFreeFreqBand {
	uint8_t device_id;
	unsigned channel;
	unsigned freq_channel_begin;
	unsigned freq_channel_end;
	unsigned threshold_rssi;
	unsigned threshold_snr;
};

struct MicUReplyGetFreeFreqBand {
	uint8_t device_id;
	bool is_left;
	uint8_t band;
	uint8_t rssi;
	uint8_t snr;
};

struct MicUSetFreqBand {
	uint8_t device_id;
	bool is_left;
	unsigned freq_band;
};

struct MicUReplySetFreqBand {
	uint8_t device_id;
	bool is_left;
	uint8_t band;
};

struct BlueReply {
	uint16_t len_msg;
	char msg[0];
};

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif//_MSC_VER

#pragma pack(pop)

}}//namespace VDProtocol::Caller
