#pragma once

#include <stdint.h>

namespace VDProtocol
{
namespace Tuner
{

enum MainOption
{
	MAIN_OPTION_MODULE,
	MAIN_OPTION_COMMAND,
	MAIN_OPTION_ADDRESS,
};

const char* const cg_main_options[] = {
	"module",
	"command",
	"address",
};

enum ModuleOption
{
	MODULE_OPTION_BASIC,
	MODULE_OPTION_OMAPL138,
	MODULE_OPTION_M5PRO,
	MODULE_OPTION_DSP1467,
	MODULE_OPTION_CONTROL_BOARD,
	MODULE_OPTION_BP1048_MASTER,
	MODULE_OPTION_BP1048_SLAVE,
	MODULE_OPTION_MP68_MASTER,
	MODULE_OPTION_MP68_SLAVE,
	MODULE_OPTION_PAD_BP1048_MASTER,
	MODULE_OPTION_PAD_BP1048_SLAVE,
	MODULE_OPTION_E5_PLUSE_BP1048,
	MODULE_OPTION_WZJ_BP1048_MASTER,
	MODULE_OPTION_WZJ_BP1048_SLAVE,
	MODULE_OPTION_DSAMP_1048_MASTER,
};

const char* const cg_module_options[] = {
	"basic",
	"OmapL138",
	"M5Pro",
	"DSP1467",
	"CTRL_BOARD",
	"BP1048_master",
	"BP1048_slave",
	"MP68_master",
	"MP68_slave",
	"pad_BP1048_master",
	"pad_BP1048_slave",
	"E5_pluse_BP1048",
	"wzj_BP1048_master",
	"wzj_BP1048_slave",
	"dsamp_BP1048_master",
};

/******************** begin 基础命令 *********************/
enum BasicCommand
{
	BASIC_COMMAND_OPEN,
	BASIC_COMMAND_CLOSE,
	BASIC_COMMAND_STATE,
	BASIC_COMMAND_QUERY_DEVICE,
	BASIC_COMMAND_TRANSFER,
};

const char* const cg_basic_commands[] = {
	"open",
	"close",
	"state",
	"QueryDevice",
	"transfer",
};

enum BasicCommandStateOption
{
	BASIC_COMMAND_STATE_OPTION_STATE,
	BASIC_COMMAND_STATE_OPTION_ERROR,
};

const char* const cg_device_command_state_option[] = {
	"state",
	"error"
};

enum BasicState
{
	BASIC_STATE_DISCONNECT,
	BASIC_STATE_CONNECT,
};

/******************** end 基础命令 *********************/

/******************** begin 模块OmapL138 *********************/
enum OmapL138Command
{
	DMAPL138_COMMAND_GET_SOFT_VERSION,
	DMAPL138_COMMAND_GET_IP,

	DMAPL138_COMMAND_SET_ANALOG_GAIN,
	DMAPL138_COMMAND_SET_DIGITAL_GAIN,
	DMAPL138_COMMAND_SET_MUTE,
	DMAPL138_COMMAND_SET_LOW_CUT_SWITCH,
	DMAPL138_COMMAND_SET_LOW_CUT_FREQ_POINT,
	DMAPL138_COMMAND_SET_COMPANDOR_SWITCH,
	DMAPL138_COMMAND_SET_COMPANDOR_GAIN,
	DMAPL138_COMMAND_SET_COMPANDOR_ATTACK,
	DMAPL138_COMMAND_SET_COMPANDOR_RELEASE,
	DMAPL138_COMMAND_SET_COMPANDOR_CURVE,
	DMAPL138_COMMAND_SAVE_COMPANDOR_CURVE_MODULE,
	DMAPL138_COMMAND_ACTIVATE_COMPANDOR_CURVE_MODULE,
	DMAPL138_COMMAND_SET_COMPANDOR_LEFT_SWITCH,
	DMAPL138_COMMAND_SET_COMPANDOR_LEFT_GAIN,
	DMAPL138_COMMAND_SET_COMPANDOR_LEFT_ATTACK,
	DMAPL138_COMMAND_SET_COMPANDOR_LEFT_RELEASE,
	DMAPL138_COMMAND_SET_COMPANDOR_LEFT_CURVE,
	DMAPL138_COMMAND_SAVE_COMPANDOR_LEFT_CURVE_MODULE,
	DMAPL138_COMMAND_ACTIVATE_COMPANDOR_LEFT_CURVE_MODULE,
	DMAPL138_COMMAND_SET_COMPANDOR_RIGHT_SWITCH,
	DMAPL138_COMMAND_SET_COMPANDOR_RIGHT_GAIN,
	DMAPL138_COMMAND_SET_COMPANDOR_RIGHT_ATTACK,
	DMAPL138_COMMAND_SET_COMPANDOR_RIGHT_RELEASE,
	DMAPL138_COMMAND_SET_COMPANDOR_RIGHT_CURVE,
	DMAPL138_COMMAND_SAVE_COMPANDOR_RIGHT_CURVE_MODULE,
	DMAPL138_COMMAND_ACTIVATE_COMPANDOR_RIGHT_CURVE_MODULE,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_SWITCH,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_UPPER,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_GAIN,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_ATTACK,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_RELEASE,
	DMAPL138_COMMAND_SET_MULTI_COMPANDOR_CURVE,
	DMAPL138_COMMAND_SET_EXCITER_COMPANDOR_SWITCH,
	DMAPL138_COMMAND_SET_EXCITER_COMPANDOR_GAIN,
	DMAPL138_COMMAND_SET_EXCITER_COMPANDOR_ATTACK,
	DMAPL138_COMMAND_SET_EXCITER_COMPANDOR_RELEASE,
	DMAPL138_COMMAND_SET_EXCITER_COMPANDOR_CURVE,
	DMAPL138_COMMAND_SET_EQ_SWITCH,
	DMAPL138_COMMAND_SET_EQ_BAND_SWITCH,
	DMAPL138_COMMAND_SET_EQ_BAND_FILTER_TYPE,
	DMAPL138_COMMAND_SET_EQ_BAND_BOOST,
	DMAPL138_COMMAND_SET_EQ_BAND_FREQ,
	DMAPL138_COMMAND_SET_EQ_BAND_Q_FACTOR,
	DMAPL138_COMMAND_SET_DELAY_SWITCH,
	DMAPL138_COMMAND_SET_DELAY,
	DMAPL138_COMMAND_SET_LIMITER_SWITCH,
	DMAPL138_COMMAND_SET_LIMITER_LEVEL,
	DMAPL138_COMMAND_SET_MIXING_MATRIX,
	DMAPL138_COMMAND_SET_SPDIF_MONITOR_GAIN,
	DMAPL138_COMMAND_SET_SPDIF_MONITOR_LINE,
	DMAPL138_COMMAND_SET_DRY_RECORDING_CHANNEL_GAIN,
	DMAPL138_COMMAND_SET_AUDIO_OUTPUT_SWITCH,
	DMAPL138_COMMAND_SET_REVERBERATOR_SWITCH,
	DMAPL138_COMMAND_SET_REVERBERATOR_MODEL,
	DMAPL138_COMMAND_SET_REVERBERATOR_PRE_DELAY,
	DMAPL138_COMMAND_SET_REVERBERATOR_TIME,
	DMAPL138_COMMAND_SET_REVERBERATOR_SEND,
	DMAPL138_COMMAND_SET_EXCITER_SWITCH,
	DMAPL138_COMMAND_SET_EXCITER_SEND,
	DMAPL138_COMMAND_SAVE_EXCITER_COEFFICIENT_MODULE,
	DMAPL138_COMMAND_ACTIVATE_EXCITER_COEFFICIENT_MODULE,
	DMAPL138_COMMAND_SET_PHASE_REVERSAL_SWITCH,

	DMAPL138_COMMAND_DOWNLOAD_FILE,
	DMAPL138_COMMAND_UPLOAD_FILE,
	DMAPL138_COMMAND_UPLOAD_FILE_RESULT,

	DMAPL138_COMMAND_DSP_VERSION,
	DMAPL138_COMMAND_DSP_INPUT_LEVEL,
	DMAPL138_COMMAND_DSP_OUTPUT_LEVEL,

	DMAPL138_COMMAND_SET_PGA2505_GAIN,
	DMAPL138_COMMAND_SET_OUTPUT_GAIN_RANGE,
	DMAPL138_COMMAND_SET_POWER_STANDBY_STATE,
	DMAPL138_COMMAND_SET_OPTICAL_INPUT_OPTION,
	DMAPL138_COMMAND_GET_PGA2505_GAIN,
	DMAPL138_COMMAND_GET_OUTPUT_GAIN_RANGE,
	DMAPL138_COMMAND_GET_POWER_STANDBY_STATE,
	DMAPL138_COMMAND_GET_OPTICAL_INPUT_OPTION,

	DMAPL138_COMMAND_POWER_OFF,
	DMAPL138_COMMAND_GET_VERSION,

	CHANNEL_PARAM_COPY,
	MODULE_PARAM_RESET,
	MONITOR_RECORD,
	SWITCH_MONO_STEREO,
	SET_RIGHT_EQ_SWITCH,
	SET_RIGHT_EQ_BAND_SWITCH,
	SET_RIGHT_EQ_BAND_FILTER_TYPE,
	SET_RIGHT_EQ_BAND_BOOST,
	SET_RIGHT_EQ_BAND_FREQ,
	SET_RIGHT_EQ_BAND_Q_FACTOR,
	SET_RIGHT_DELAY_SWITCH,
	SET_RIGHT_DELAY,
	SET_RIGHT_MUTE,
	SET_RIGHT_PHASE_REVERSAL_SWITCH,
	SET_RIGHT_DIGITAL_GAIN,
	SET_RIGHT_LIMITER_SWITCH,
	SET_RIGHT_LIMITER_LEVEL,
	SET_RIGHT_MIXING_MATRIX,
	SET_RIGHT_COMPANDOR_SWITCH,
	SET_RIGHT_COMPANDOR_GAIN,
	SET_RIGHT_COMPANDOR_ATTACK,
	SET_RIGHT_COMPANDOR_RELEASE,
	SET_RIGHT_COMPANDOR_CURVE,
	SET_RIGHT_MULTI_COMPANDOR_SWITCH,
	SET_RIGHT_MULTI_COMPANDOR_UPPER,
	SET_RIGHT_MULTI_COMPANDOR_GAIN,
	SET_RIGHT_MULTI_COMPANDOR_ATTACK,
	SET_RIGHT_MULTI_COMPANDOR_RELEASE,
	SET_RIGHT_MULTI_COMPANDOR_CURVE,

	SET_FREQ_SHIFT_SWITCH,
	SET_FREQ_SHIFT_FREQ,

	SET_SUBWOOFER_SWITCH,
};

const char* const cg_omapl138_command[] = {
	"GetSoftVersion",
	"GetIP",

	"SetAnalogGain",
	"SetDigitalGain",
	"SetMute",
	"SetLowCutSwitch",
	"SetLowCutFreqPoint",
	"SetCompandorSwitch",
	"SetCompandorGain",
	"SetCompandorAttack",
	"SetCompandorRelease",
	"SetCompandorCurve",
	"SaveCompandorCurveModule",
	"ActivateCompandorCurveModule",
	"SetCompandorLeftSwitch",
	"SetCompandorLeftGain",
	"SetCompandorLeftAttack",
	"SetCompandorLeftRelease",
	"SetCompandorLeftCurve",
	"SaveCompandorLeftCurveModule",
	"ActivateCompandorLeftCurveModule",
	"SetCompandorRightSwitch",
	"SetCompandorRightGain",
	"SetCompandorRightAttack",
	"SetCompandorRightRelease",
	"SetCompandorRightCurve",
	"SaveCompandorRightCurveModule",
	"ActivateCompandorRightCurveModule",
	"SetMultiCompandorSwitch",
	"SetMultiCompandorUpper",
	"SetMultiCompandorGain",
	"SetMultiCompandorAttack",
	"SetMultiCompandorRelease",
	"SetMultiCompandorCurve",
	"SetExciterCompandorSwitch",
	"SetExciterCompandorGain",
	"SetExciterCompandorAttack",
	"SetExciterCompandorRelease",
	"SetExciterCompandorCurve",
	"SetEQSwitch",
	"SetEQBandSwitch",
	"SetEQBandFilterType",
	"SetEQBandBoost",
	"SetEQBandFreq",
	"SetEQBandQFactor",
	"SetDelaySwitch",
	"SetDelay",
	"SetLimiterSwitch",
	"SetLimiterLevel",
	"SetMixingMatrix",
	"SetSPDIFMonitorGain",
	"SetSPDIFMonitorLine",
	"SetDryRecordingChannelGain",
	"SetAudioOutputSwitch",
	"SetReverberatorSwitch",
	"SetReverberatorModel",
	"SetReverberatorPreDelay",
	"SetReverberatorTime",
	"SetReverberatorSend",
	"SetExciterSwitch",
	"SetExciterSend",
	"SaveExciterCoefficientModule",
	"ActivateExciterCoefficientModule",
	"SetPhaseReversalSwitch",

	"DownloadFile",
	"UploadFile",
	"UploadFileResult",

	"DspVersion",
	"DspInputLevel",
	"DspOutputLevel",

	"SetPGA2505Gain",
	"SetOutputGainRange",
	"SetPowerStandbyState",
	"SetOpticalInputOption",
	"GetPGA2505Gain",
	"GetOutputGainRange",
	"GetPowerStandbyState",
	"GetOpticalInputOption",

	"PowerOff",
	"GetVersion",

	"ChannelParamCopy",
	"ModuleParamReset",
	"MonitorRecord",
	"SwitchMonoStereo",
	"SetRightEQSwitch",
	"SetRightEQBandSwitch",
	"SetRightEQBandFilterType",
	"SetRightEQBandBoost",
	"SetRightEQBandFreq",
	"SetRightEQBandQFactor",
	"SetRightDelaySwitch",
	"SetRightDelay",
	"SetRightMute",
	"SetRightPhaseReversalSwitch",
	"SetRightDigitalGain",
	"SetRightLimiterSwitch",
	"SetRightLimiterLevel",
	"SetRightMixingMatrix",
	"SetRightCompandorSwitch",
	"SetRightCompandorGain",
	"SetRightCompandorAttack",
	"SetRightCompandorRelease",
	"SetRightCompandorCurve",
	"SetRightMultiCompandorSwitch",
	"SetRightMultiCompandorUpper",
	"SetRightMultiCompandorGain",
	"SetRightMultiCompandorAttack",
	"SetRightMultiCompandorRelease",
	"SetRightMultiCompandorCurve",

	"SetFreqShiftSwitch",
	"SetFreqShiftFreq",

	"SetSubwooferSwitch",
};
/******************** end 模块OmapL138 *********************/

/******************** begin 模块M5Pro *********************/
enum M5proCommand
{
	M5PRO_COMMAND_MUSIC_SET_EQ_BAND,
	M5PRO_COMMAND_MUSIC_SET_EQ_PASS,
	M5PRO_COMMAND_MIC_SET_HPF_FREQ,
	M5PRO_COMMAND_MIC_SET_FBE,
	M5PRO_COMMAND_MIC_SET_AUTO_MUTE,
	M5PRO_COMMAND_MIC_SET_EQ_BAND,
	M5PRO_COMMAND_MIC_SET_EQ_PASS,
	M5PRO_COMMAND_MUSIC_DEQ_SET_EQ_BAND_PASS,
	M5PRO_COMMAND_MUSIC_DEQ_SET_EQ_BAND,
	M5PRO_COMMAND_MUSIC_DEQ_SET_WORK_MODE,
	M5PRO_COMMAND_MUSIC_DEQ_SET_ATTACK,
	M5PRO_COMMAND_MUSIC_DEQ_SET_RELEASE,
	M5PRO_COMMAND_MUSIC_DEQ_SET_START_LEVEL,
	M5PRO_COMMAND_MUSIC_DEQ_SET_DYNAMIC_SCALE,
	M5PRO_COMMAND_MUSIC_DEQ_SET_DYNAMIC_MODE,
	M5PRO_COMMAND_MIC_DEQ_SET_EQ_BAND_PASS,
	M5PRO_COMMAND_MIC_DEQ_SET_EQ_BAND,
	M5PRO_COMMAND_MIC_DEQ_SET_WORK_MODE,
	M5PRO_COMMAND_MIC_DEQ_SET_ATTACK,
	M5PRO_COMMAND_MIC_DEQ_SET_RELEASE,
	M5PRO_COMMAND_MIC_DEQ_SET_START_LEVEL,
	M5PRO_COMMAND_MIC_DEQ_SET_DYNAMIC_SCALE,
	M5PRO_COMMAND_MIC_DEQ_SET_DYNAMIC_MODE,
	M5PRO_COMMAND_ECHO_EFFECT_SET_LEVEL_PHASE,
	M5PRO_COMMAND_ECHO_EFFECT_SET_LEVEL,
	M5PRO_COMMAND_ECHO_EFFECT_SET_DIRECT_LEVEL_PHASE,
	M5PRO_COMMAND_ECHO_EFFECT_SET_DIRECT_LEVEL,
	M5PRO_COMMAND_ECHO_EFFECT_SET_HPF,
	M5PRO_COMMAND_ECHO_EFFECT_SET_LPF,
	M5PRO_COMMAND_ECHO_EFFECT_SET_PRE_DELAY,
	M5PRO_COMMAND_ECHO_EFFECT_SET_DELAY,
	M5PRO_COMMAND_ECHO_EFFECT_SET_REPEAT,
	M5PRO_COMMAND_ECHO_EFFECT_SET_RCH_DELAY,
	M5PRO_COMMAND_ECHO_EFFECT_SET_RCH_PRE_DELAY,
	M5PRO_COMMAND_REVERB_SET_MIC_LEVEL_PHASE,
	M5PRO_COMMAND_REVERB_SET_MIC_LEVEL,
	M5PRO_COMMAND_REVERB_SET_DIRECT_MIC_LEVEL_PHASE,
	M5PRO_COMMAND_REVERB_SET_DIRECT_MIC_LEVEL,
	M5PRO_COMMAND_REVERB_SET_DELAY,
	M5PRO_COMMAND_REVERB_SET_PRE_DELAY,
	M5PRO_COMMAND_REVERB_SET_MIC_HPF,
	M5PRO_COMMAND_REVERB_SET_MIC_LPF,
	M5PRO_COMMAND_REVERB_SET_MUS_LEVEL_PHASE,
	M5PRO_COMMAND_REVERB_SET_MUS_LEVEL,
	M5PRO_COMMAND_REVERB_SET_MUS_HPF,
	M5PRO_COMMAND_REVERB_SET_MUS_LPF,
	M5PRO_COMMAND_REVERB_SET_HIGH_VOL_ATTEN,
	M5PRO_COMMAND_REVERB_SET_LOW_VOL_ATTEN,
	M5PRO_COMMAND_OUTPUT_SET_MUSIC_LEVEL_SING,
	M5PRO_COMMAND_OUTPUT_SET_MUSIC_LEVEL_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_MUSIC_LEVEL_PHASE_SING,
	M5PRO_COMMAND_OUTPUT_SET_MUSIC_LEVEL_PHASE_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_MIC_LEVEL_SING,
	M5PRO_COMMAND_OUTPUT_SET_MIC_LEVEL_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_MIC_LEVEL_PHASE_SING,
	M5PRO_COMMAND_OUTPUT_SET_MIC_LEVEL_PHASE_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_ECHO_LEVEL_SING,
	M5PRO_COMMAND_OUTPUT_SET_ECHO_LEVEL_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_ECHO_LEVEL_PHASE_SING,
	M5PRO_COMMAND_OUTPUT_SET_ECHO_LEVEL_PHASE_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_REVERB_LEVEL_SING,
	M5PRO_COMMAND_OUTPUT_SET_REVERB_LEVEL_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_REVERB_LEVEL_PHASE_SING,
	M5PRO_COMMAND_OUTPUT_SET_REVERB_LEVEL_PHASE_DISCO,
	M5PRO_COMMAND_OUTPUT_SET_HPF_TYPE,
	M5PRO_COMMAND_OUTPUT_SET_HPF_FREQ,
	M5PRO_COMMAND_OUTPUT_SET_HPF_Q,
	M5PRO_COMMAND_OUTPUT_SET_LPF_TYPE,
	M5PRO_COMMAND_OUTPUT_SET_LPF_FREQ,
	M5PRO_COMMAND_OUTPUT_SET_LPF_Q,
	M5PRO_COMMAND_OUTPUT_SET_ATTACK,
	M5PRO_COMMAND_OUTPUT_SET_RELEASE,
	M5PRO_COMMAND_OUTPUT_SET_START_LEVEL,
	M5PRO_COMMAND_OUTPUT_SET_COMPRESS_LIMIT_SCALE,
	M5PRO_COMMAND_OUTPUT_SET_COMPRESS_LIMIT_MODE,
	M5PRO_COMMAND_OUTPUT_SET_LEFT_CHANNEL_DELAY,
	M5PRO_COMMAND_OUTPUT_SET_RIGHT_CHANNEL_DELAY,
	M5PRO_COMMAND_OUTPUT_SET_DELAY,
	M5PRO_COMMAND_OUTPUT_SET_LEFT_CHANNEL_MUTE,
	M5PRO_COMMAND_OUTPUT_SET_RIGHT_CHANNEL_MUTE,
	M5PRO_COMMAND_OUTPUT_SET_MUTE,
	M5PRO_COMMAND_OUTPUT_SET_EQ_CONTROL_MODE,
	M5PRO_COMMAND_OUTPUT_SET_EQ_PASS,
	M5PRO_COMMAND_OUTPUT_SET_EQ_BAND,
	M5PRO_COMMAND_SUR_OUT_SET_MIC_VOLUME,
	M5PRO_COMMAND_SUR_OUT_SET_GUITAR_VOLUME,
	M5PRO_COMMAND_SUR_OUT_SET_GUITAR_TO_MIC_VOLUME,
	M5PRO_COMMAND_SYS_GET_MAIN_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_SET_MAIN_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_SET_GUITAR_VOLUME,
	M5PRO_COMMAND_SYS_GET_GUITAR_VOLUME,
	M5PRO_COMMAND_SYS_GET_MUSIC_VOLUME,
	M5PRO_COMMAND_SYS_SET_MUSIC_VOLUME,
	M5PRO_COMMAND_SYS_GET_AUX2_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_SET_AUX2_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_SET_AUX1_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_GET_AUX1_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_GET_MIC_VOLUME,
	M5PRO_COMMAND_SYS_SET_MIC_VOLUME,
	M5PRO_COMMAND_SYS_SET_SUR_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_GET_SUR_GUITAR_LEVEL,
	M5PRO_COMMAND_SYS_GET_EFFECT_VOLUME,
	M5PRO_COMMAND_SYS_SET_EFFECT_VOLUME,
	M5PRO_COMMAND_SYS_SET_DEV_MODE_SAVE,
	M5PRO_COMMAND_SYS_SET_DEV_MODE_INVOKE,
	M5PRO_COMMAND_SYS_GET_MODULE_DATAS,
	M5PRO_COMMAND_SYS_UPLOAD_MODULE_DATAS,
	M5PRO_COMMAND_SYS_MGR_SET_AUTH_MODE,
};

const char* const cg_m5pro_command[] = {
	"MusicSetCurEqInfo",
	"MusicSetEqPass",
	"MicrophoneSetHPFFreq",
	"MicrophoneSetFBE",
	"MicrophoneSetAutoMute",
	"MicrophoneSetCurEqInfo",
	"MicrophoneSetPass",
	"MusicDeqSetEQBandPass",
	"MusicDeqSetEqBand",
	"MusicDeqSetWorkMode",
	"MusicDeqSetAttack",
	"MusicDeqSetRelease",
	"MusicDeqSetStartLevel",
	"MusicDeqSetDynamicScale",
	"MusicDeqSetDynamicMode",
	"MicDeqSetEQBandPass",
	"MicDeqSetEqBand",
	"MicDeqSetWorkMode",
	"MicDeqSetAttack",
	"MicDeqSetRelease",
	"MicDeqSetStartLevel",
	"MicDeqSetDynamicScale",
	"MicDeqSetDynamicMode",
	"EchoEffectSetLevelPhase",
	"EchoEffectSetLevel",
	"EchoEffectSetDirectLevelPhase",
	"EchoEffectSetDirectLevel",
	"EchoEffectSetHPF",
	"EchoEffectSetLPF",
	"EchoEffectSetPreDelay",
	"EchoEffectSetDelay",
	"EchoEffectSetRepeat",
	"EchoEffectSetRChDelay",
	"EchoEffectSetRChPreDelay",
	"ReverbEffectSetMicLevelPhase",
	"ReverbEffectSetMicLevel",
	"ReverbEffectSetDirectMicLevelPhase",
	"ReverbEffectSetDirectMicLevel",
	"ReverbEffectSetDelay",
	"ReverbEffectSetPreDelay",
	"ReverbEffectSetMicHPF",
	"ReverbEffectSetMicLPF",
	"ReverbEffectSetMusLevelPhase",
	"ReverbEffectSetMusLevel",
	"ReverbEffectSetMusHPF",
	"ReverbEffectSetMusLPF",
	"ReverbEffectSetHighVolAtten",
	"ReverbEffectSetLowVolAtten",
	"OutputSetMusicLevelSing",
	"OutputSetMusicLevelDisco",
	"OutputSetMusicLevelPhaseSing",
	"OutputSetMusicLevelPhaseDisco",
	"OutputSetMicLevelSing",
	"OutputSetMicLevelDisco",
	"OutputSetMicLevelPhaseSing",
	"OutputSetMicLevelPhaseDisco",
	"OutputSetEchoLevelSing",
	"OutputSetEchoLevelDisco",
	"OutputSetEchoLevelPhaseSing",
	"OutputSetEchoLevelPhaseDisco",
	"OutputSetReverbLevelSing",
	"OutputSetReverbLevelDisco",
	"OutputSetReverbLevelPhaseSing",
	"OutputSetReverbLevelPhaseDisco",
	"OutputSetHPFType",
	"OutputSetHPFFreq",
	"OutputSetHPFQ",
	"OutputSetLPFType",
	"OutputSetLPFFreq",
	"OutputSetLPFQ",
	"OutputSetAttackTime",
	"OutputSetReleaseTime",
	"OutputSetStartLevel",
	"OutputSetCompressLimitScale",
	"OutputSetCompressLimitMode",
	"OutputSetLeftChannelDelay",
	"OutputSetRightChannelDelay",
	"OutputSetDelay",
	"OutputSetLeftChannelMute",
	"OutputSetRightChannelMute",
	"OutputSetMute",
	"OutputSetEqControlMode",
	"OutputSetCurEQPass",
	"OutputSetEqInfo",
	"SurOutSetMic4Volume",
	"SurOutSetGuitar4Volume",
	"SurOutSetGuitarToMicVolume",
	"SysMgrGetMainGuitarLevel",
	"SysMgrSetMainGuitarLevel",
	"SysMgrSetGuitarVolume",
	"SysMgrGetGuitarVolume",
	"SysMgrGetMusicVolume",
	"SysMgrSetMusicVolume",
	"SysMgrGetAux2GuitarLevel",
	"SysMgrSetAux2GuitarLevel",
	"SysMgrSetAux1GuitarLevel",
	"SysMgrGetAux1GuitarLevel",
	"SysMgrGetMicVolume",
	"SysMgrSetMicVolume",
	"SysMgrSetSurGuitarLevel",
	"SysMgrGetSurGuitarLevel",
	"SysMgrGetEffectVolume",
	"SysMgrSetEffectVolume",
	"SysMgrSetDevModeSave",
	"SysMgrSetDevModeInvoke",
	"SysMgrGetModuleDatas",
	"SysMgrDownloadSetting2Dev",
	"SysMgrSetAuthMode",
};
/******************** end 模块M5Pro *********************/

/******************** begin 模块DSP1467 *********************/
enum DSP1467Command
{
	DSP1467_COMMAND_QUERY_CHANNEL_VOLUME,
	DSP1467_COMMAND_SET_CHANNEL_VOLUME,
	DSP1467_COMMAND_QUERY_MIXING_INPUT_OUTPUT,
	DSP1467_COMMAND_SET_MIXING_INPUT_OUTPUT,
};

const char* const cg_dsp1467_command[] = {
	"QueryChannelVolume",
	"SetChannelVolume",
	"QueryMixingInputOutput",
	"SetMixingInputOutput",
};
/******************** end 模块DSP1467 *********************/

/******************** begin 模块主控台 *********************/
enum ControlBoardCommand
{
	CONTROL_BOARD_COMMAND_CHANNEL_VOLUME,
};

const char* const cg_ctrl_board_command[] = {
	"ChannelVolume",
};
/******************** end 模块主控台 *********************/

/******************** begin 模块BP1048 *********************/
enum BP1048Command
{
	BP1048_COMMAND_SET_PARAMETERS,
	BP1048_COMMAND_GET_PARAMETERS,

	BP1048_COMMAND_SET_ADC_MODE,
	BP1048_COMMAND_SET_ADC_MUTE,
	BP1048_COMMAND_SET_ADC_VOLUME,
	BP1048_COMMAND_SET_ADC_SAMPLE_RATE,
	BP1048_COMMAND_SET_ADC_LR_SWAP,
	BP1048_COMMAND_SET_ADC_DC_BLOCKER_COEFFICIENT,
	BP1048_COMMAND_SET_ADC_FADE_TIME,
	BP1048_COMMAND_SET_ADC_MCLK_SOURCE,
	BP1048_COMMAND_SET_ADC_DC_BLOCKER_ENABLE,

	BP1048_COMMAND_SET_AGC_MODE,
	BP1048_COMMAND_SET_AGC_MAX_LEVEL,
	BP1048_COMMAND_SET_AGC_TARGET_LEVEL,
	BP1048_COMMAND_SET_AGC_MAX_GAIN,
	BP1048_COMMAND_SET_AGC_MIN_GAIN,
	BP1048_COMMAND_SET_AGC_GAIN_OFFSET,
	BP1048_COMMAND_SET_AGC_FRAME_TIME,
	BP1048_COMMAND_SET_AGC_HOLD_N_FRAME_TIME,
	BP1048_COMMAND_SET_AGC_ATTACK_TIME,
	BP1048_COMMAND_SET_AGC_DECAY_TIME,
	BP1048_COMMAND_SET_AGC_NOISE_GATE_ENABLE,
	BP1048_COMMAND_SET_AGC_NOISE_GATE_THRESHOLD,
	BP1048_COMMAND_SET_AGC_NOISE_GATE_MODE,
	BP1048_COMMAND_SET_AGC_NOISE_HOLD_N_FRAME_TIME,

	BP1048_COMMAND_SET_AUTO_TUNE_ENABLE,
	BP1048_COMMAND_SET_AUTO_TUNE_KEY,
	BP1048_COMMAND_SET_AUTO_TUNE_SNAP,
	BP1048_COMMAND_SET_AUTO_TUNE_PITCH_ACCURACY,

	BP1048_COMMAND_SET_ECHO_ENABLE,
	BP1048_COMMAND_SET_ECHO_CUTOFF_FREQ,
	BP1048_COMMAND_SET_ECHO_ATTENUATION,
	BP1048_COMMAND_SET_ECHO_DELAY,
	BP1048_COMMAND_SET_ECHO_DIRECT,
	BP1048_COMMAND_SET_ECHO_MAX_DELAY,

	BP1048_COMMAND_SET_NOISE_SUPPRESSOR_ENABLE,
	BP1048_COMMAND_SET_NOISE_SUPPRESSOR_THRESHOLD,
	BP1048_COMMAND_SET_NOISE_SUPPRESSOR_RATIO,
	BP1048_COMMAND_SET_NOISE_SUPPRESSOR_ATTACK,
	BP1048_COMMAND_SET_NOISE_SUPPRESSOR_RELEASE,

	BP1048_COMMAND_SET_HOWLING_CTRL_ENABLE,
	BP1048_COMMAND_SET_HOWLING_CTRL_MODE,

	BP1048_COMMAND_SET_SILENCE_DETECTOR_ENABLE,
	BP1048_COMMAND_SET_SILENCE_DETECTOR_AMPLITUDE,

	BP1048_COMMAND_SET_FREQ_SHIFTER_ENABLE,
	BP1048_COMMAND_SET_FREQ_SHIFTER_DELTA,

	BP1048_COMMAND_SET_PITCH_SHIFTER_ENABLE,
	BP1048_COMMAND_SET_PITCH_SHIFTER_KEY,
	
	BP1048_COMMAND_SET_PITCH_SHIFTER_PRO_ENABLE,
	BP1048_COMMAND_SET_PITCH_SHIFTER_PRO_KEY,

	BP1048_COMMAND_SET_REVERB_ENABLE,
	BP1048_COMMAND_SET_REVERB_DRY,
	BP1048_COMMAND_SET_REVERB_WET,
	BP1048_COMMAND_SET_REVERB_WIDTH,
	BP1048_COMMAND_SET_REVERB_ROOM,
	BP1048_COMMAND_SET_REVERB_DAMPING,
	BP1048_COMMAND_SET_REVERB_MONO,

	BP1048_COMMAND_SET_REVERB_PLATE_ENABLE,
	BP1048_COMMAND_SET_REVERB_PLATE_FREQ,
	BP1048_COMMAND_SET_REVERB_PLATE_MODULATION,
	BP1048_COMMAND_SET_REVERB_PLATE_PREDELAY,
	BP1048_COMMAND_SET_REVERB_PLATE_DIFFUSION,
	BP1048_COMMAND_SET_REVERB_PLATE_DECAY,
	BP1048_COMMAND_SET_REVERB_PLATE_DAMPING,
	BP1048_COMMAND_SET_REVERB_PLATE_WET_DRY_MIX,

	BP1048_COMMAND_SET_REVERB_PRO_ENABLE,
	BP1048_COMMAND_SET_REVERB_PRO_DRY,
	BP1048_COMMAND_SET_REVERB_PRO_WET,
	BP1048_COMMAND_SET_REVERB_PRO_ER_WET,
	BP1048_COMMAND_SET_REVERB_PRO_ER_FACTOR,
	BP1048_COMMAND_SET_REVERB_PRO_ER_WIDTH,
	BP1048_COMMAND_SET_REVERB_PRO_ER_TOLATE,
	BP1048_COMMAND_SET_REVERB_PRO_RT60,
	BP1048_COMMAND_SET_REVERB_PRO_DELAY,
	BP1048_COMMAND_SET_REVERB_PRO_WIDTH,
	BP1048_COMMAND_SET_REVERB_PRO_WANDER,
	BP1048_COMMAND_SET_REVERB_PRO_SPIN,
	BP1048_COMMAND_SET_REVERB_PRO_INPUT_LPF,
	BP1048_COMMAND_SET_REVERB_PRO_DAMP_LPF,
	BP1048_COMMAND_SET_REVERB_PRO_BASS_LPF,
	BP1048_COMMAND_SET_REVERB_PRO_BASS_B,
	BP1048_COMMAND_SET_REVERB_PRO_OUTPUT_LPF,

	BP1048_COMMAND_SET_MV_3D_ENABLE,
	BP1048_COMMAND_SET_MV_3D_INTENSITY,

	BP1048_COMMAND_SET_MV_3D_PLUS_ENABLE,
	BP1048_COMMAND_SET_MV_3D_PLUS_INTENSITY,

	BP1048_COMMAND_SET_MV_BASS_ENABLE,
	BP1048_COMMAND_SET_MV_BASS_CUTOFF_FREQ,
	BP1048_COMMAND_SET_MV_BASS_INTENSITY,
	BP1048_COMMAND_SET_MV_BASS_ENHANCED,

	BP1048_COMMAND_SET_MV_BASS_CLASSIC_ENABLE,
	BP1048_COMMAND_SET_MV_BASS_CLASSIC_CUTOFF_FREQ,
	BP1048_COMMAND_SET_MV_BASS_CLASSIC_INTENSITY,

	BP1048_COMMAND_SET_VOICE_CHANGER_ENABLE,
	BP1048_COMMAND_SET_VOICE_CHANGER_PITCH,
	BP1048_COMMAND_SET_VOICE_CHANGER_FORMANT,

	BP1048_COMMAND_SET_VOICE_CHANGER_PRO_ENABLE,
	BP1048_COMMAND_SET_VOICE_CHANGER_PRO_PITCH,
	BP1048_COMMAND_SET_VOICE_CHANGER_PRO_FORMANT,

	BP1048_COMMAND_SET_VOCAL_CUT_ENABLE,
	BP1048_COMMAND_SET_VOCAL_CUT_WET_DRY_MIX,

	BP1048_COMMAND_SET_VOCAL_REMOVER_ENABLE,
	BP1048_COMMAND_SET_VOCAL_REMOVER_LOWER_FREQ,
	BP1048_COMMAND_SET_VOCAL_REMOVER_HIGHER_FREQ,

	BP1048_COMMAND_SET_PHASE_CTRL_ENABLE,
	BP1048_COMMAND_SET_PHASE_CTRL_PHASE_DIFFERENCE,

	BP1048_COMMAND_SET_PCM_DELAY_ENABLE,
	BP1048_COMMAND_SET_PCM_DELAY_TIME,
	BP1048_COMMAND_SET_PCM_DELAY_MAX_DELAY,
	BP1048_COMMAND_SET_PCM_DELAY_HIGH_QUALITY_ENABLE,

	BP1048_COMMAND_SET_HARMONIC_EXCITER_ENABLE,
	BP1048_COMMAND_SET_HARMONIC_EXCITER_CUTOFF_FREQ,
	BP1048_COMMAND_SET_HARMONIC_EXCITER_DRY,
	BP1048_COMMAND_SET_HARMONIC_EXCITER_WET,

	BP1048_COMMAND_SET_PINGPONG_ENABLE,
	BP1048_COMMAND_SET_PINGPONG_ATTENUATION,
	BP1048_COMMAND_SET_PINGPONG_DELAY,
	BP1048_COMMAND_SET_PINGPONG_HIGH_QUALITY_ENABLE,
	BP1048_COMMAND_SET_PINGPONG_WET_DRY_MIX,
	BP1048_COMMAND_SET_PINGPONG_MAX_DELAY,

	BP1048_COMMAND_SET_CHORUS_ENABLE,
	BP1048_COMMAND_SET_CHORUS_DELAY,
	BP1048_COMMAND_SET_CHORUS_MODULATION_DEPTH,
	BP1048_COMMAND_SET_CHORUS_MODULATION_RATE,
	BP1048_COMMAND_SET_CHORUS_FEEDBACK,
	BP1048_COMMAND_SET_CHORUS_DRY,
	BP1048_COMMAND_SET_CHORUS_WET,

	BP1048_COMMAND_SET_AUTO_WAH_ENABLE,
	BP1048_COMMAND_SET_AUTO_WAH_MODULATION_RATE,
	BP1048_COMMAND_SET_AUTO_WAH_MIN_FREQ,
	BP1048_COMMAND_SET_AUTO_WAH_MAX_FREQ,
	BP1048_COMMAND_SET_AUTO_WAH_DEPTH,
	BP1048_COMMAND_SET_AUTO_WAH_DRY,
	BP1048_COMMAND_SET_AUTO_WAH_WET,

	BP1048_COMMAND_SET_STEREO_WIDENER_ENABLE,
	BP1048_COMMAND_SET_STEREO_WIDENER_SHAPING,

	BP1048_COMMAND_SET_GAIN_ENABLE,
	BP1048_COMMAND_SET_GAIN_MUTE,
	BP1048_COMMAND_SET_GAIN,

	BP1048_COMMAND_SET_DRC_ENABLE,
	BP1048_COMMAND_SET_DRC_CROSSOVER_FREQ,
	BP1048_COMMAND_SET_DRC_MODE,
	BP1048_COMMAND_SET_DRC_Q,
	BP1048_COMMAND_SET_DRC_THRESHOLD,
	BP1048_COMMAND_SET_DRC_RATIO,
	BP1048_COMMAND_SET_DRC_ATTACK,
	BP1048_COMMAND_SET_DRC_RELEASE,
	BP1048_COMMAND_SET_DRC_PREGAIN1,
	BP1048_COMMAND_SET_DRC_PREGAIN2,

	BP1048_COMMAND_SET_EQ_ENABLE,
	BP1048_COMMAND_SET_EQ_PREGAIN,
	BP1048_COMMAND_SET_EQ_FILTER_ENABLE,
	BP1048_COMMAND_SET_EQ_FILTER_TYPE,
	BP1048_COMMAND_SET_EQ_FILTER_FREQ,
	BP1048_COMMAND_SET_EQ_FILTER_Q,
	BP1048_COMMAND_SET_EQ_FILTER_BOOST,

	BP1048_COMMAND_GET_ANALOG_OUTPUT_MODE,
	BP1048_COMMAND_SET_ANALOG_OUTPUT_MODE,

	BP1048_COMMAND_GET_GAIN_LIMIT,
	BP1048_COMMAND_SET_GAIN_LIMIT,

	BP1048_NOTIFY_SYSTEM_STATUS,
	BP1048_NOTIFY_ADC_PARAMETERS,
	BP1048_NOTIFY_AGC_PARAMETERS,
	BP1048_NOTIFY_AUTO_TUNE_PARAMETERS,
	BP1048_NOTIFY_ECHO_PARAMETERS,
	BP1048_NOTIFY_NOISE_SUPPRESSOR_PARAMETERS,
	BP1048_NOTIFY_FREQ_SHIFTER_PARAMETERS,
	BP1048_NOTIFY_HOWLING_CTRL_PARAMETERS,
	BP1048_NOTIFY_PITCH_SHIFTER_PARAMETERS,
	BP1048_NOTIFY_REVERB_PARAMETERS,
	BP1048_NOTIFY_SILENCE_DETECTOR_PARAMETERS,
	BP1048_NOTIFY_MV_3D_PARAMETERS,
	BP1048_NOTIFY_MV_BASS_PARAMETERS,
	BP1048_NOTIFY_VOICE_CHANGER_PARAMETERS,
	BP1048_NOTIFY_VOCAL_CUT_PARAMETERS,
	BP1048_NOTIFY_REVERB_PLATE_PARAMETERS,
	BP1048_NOTIFY_REVERB_PRO_PARAMETERS,
	BP1048_NOTIFY_VOICE_CHANGER_PRO_PARAMETERS,
	BP1048_NOTIFY_PHASE_CTRL_PARAMETERS,
	BP1048_NOTIFY_VOCAL_REMOVER_PARAMETERS,
	BP1048_NOTIFY_PITCH_SHIFTER_PRO_PARAMETERS,
	BP1048_NOTIFY_MV_BASS_CLASSIC_PARAMETERS,
	BP1048_NOTIFY_PCM_DELAY_PARAMETERS,
	BP1048_NOTIFY_HARMONIC_EXCITER_PARAMETERS,
	BP1048_NOTIFY_CHORUS_PARAMETERS,
	BP1048_NOTIFY_AUTO_WAH_PARAMETERS,
	BP1048_NOTIFY_STEREO_WIDENER_PARAMETERS,
	BP1048_NOTIFY_PINGPONG_PARAMETERS,
	BP1048_NOTIFY_MV_3D_PLUS_PARAMETERS,
	BP1048_NOTIFY_GAIN_PARAMETERS,
	BP1048_NOTIFY_DRC_PARAMETERS,
	BP1048_NOTIFY_EQ_PARAMETERS,

	BP1048_COMMAND_SET_RMS_DRC_ENABLE,
	BP1048_COMMAND_SET_RMS_DRC_ATTACK,
	BP1048_COMMAND_SET_RMS_DRC_RELEASE,
	BP1048_COMMAND_SET_RMS_DRC_GAIN,
	BP1048_COMMAND_SET_RMS_DRC_NUMBER_POINT,
	BP1048_COMMAND_SET_RMS_DRC_POINT,
	BP1048_COMMAND_SET_RMS_DRC_MODULE_ID,
	BP1048_COMMAND_SAVE_RMS_DRC_MODULE,

	BP1048_NOTIFY_RMS_DRC_PARAMETERS,

	BP1048_COMMAND_GET_SYSTEM_AUDIO_SOURCE,
	BP1048_COMMAND_SET_SYSTEM_AUDIO_SOURCE,

	BP1048_COMMAND_GET_FM_FREQ_BAND,
	BP1048_COMMAND_GET_FM_CHANNEL_SPACE,
	BP1048_COMMAND_GET_FM_SOUND_CHANNEL_MODE,
	BP1048_COMMAND_SET_FM_SOUND_CHANNEL_MODE,
	BP1048_COMMAND_GET_FM_BASS_BOOST,
	BP1048_COMMAND_SET_FM_BASS_BOOST,
	BP1048_COMMAND_GET_FM_DE_EMPHASIS,
	BP1048_COMMAND_GET_FM_RDS_EN,
	BP1048_COMMAND_FM_AUTO_SEARCH,
	BP1048_COMMAND_FM_MANUAL_SEARCH,
	BP1048_COMMAND_SET_FM_CHANNEL_INDEX,
	BP1048_COMMAND_GET_FM_CHANNEL_INDEX,
	BP1048_COMMAND_SET_FM_FREQ,
	BP1048_COMMAND_GET_FM_FREQ,
	BP1048_COMMAND_GET_FM_STATE,
	BP1048_COMMAND_GET_FM_STORE_COUNT,
	BP1048_COMMAND_GET_FM_STORE_LIST,

	BP1048_NOTIFY_SYSTEM_AUDIO_SOURCE,
	BP1048_NOTIFY_FM_FREQ_BAND,
	BP1048_NOTIFY_FM_CHANNEL_SPACE,
	BP1048_NOTIFY_FM_SOUND_CHANNEL_MODE,
	BP1048_NOTIFY_FM_BASS_BOOST,
	BP1048_NOTIFY_FM_DE_EMPHASIS,
	BP1048_NOTIFY_FM_RDS_EN,
	BP1048_NOTIFY_FM_CHANNEL_INDEX,
	BP1048_NOTIFY_FM_FREQ,
	BP1048_NOTIFY_FM_STATE,
	BP1048_NOTIFY_FM_STORE_COUNT,
	BP1048_NOTIFY_FM_STORE_LIST,

	BP1048_COMMAND_SET_AIMY_EQ_ENABLE,
	BP1048_COMMAND_SET_AIMY_EQ_PREGAIN,
	BP1048_COMMAND_SET_AIMY_EQ_FILTER_ENABLE,
	BP1048_COMMAND_SET_AIMY_EQ_FILTER_TYPE,
	BP1048_COMMAND_SET_AIMY_EQ_FILTER_FREQ,
	BP1048_COMMAND_SET_AIMY_EQ_FILTER_Q,
	BP1048_COMMAND_SET_AIMY_EQ_FILTER_BOOST,

	BP1048_NOTIFY_AIMY_EQ_PARAMETERS,

	BP1048_COMMAND_SET_AIMY_PCM_DELAY_ENABLE,
	BP1048_COMMAND_SET_AIMY_PCM_DELAY_LEFT,
	BP1048_COMMAND_SET_AIMY_PCM_DELAY_RIGHT,
	BP1048_COMMAND_SET_AIMY_PCM_DELAY_MAX_DELAY,

	BP1048_COMMAND_SET_AIMY_PHASE_CTRL_ENABLE,
	BP1048_COMMAND_SET_AIMY_PHASE_CTRL_PHASE_DIFFERENCE,
	BP1048_COMMAND_SET_AIMY_PHASE_CTRL_CHANNEL,

	BP1048_NOTIFY_AIMY_PCM_DELAY_PARAMETERS,
	BP1048_NOTIFY_AIMY_PHASE_CTRL_PARAMETERS,

	BP1048_COMMAND_SAVE_PARAMETERS,
	BP1048_NOTIFY_SAVE_PARAMETERS,

	BP1048_COMMAND_GET_EFFECT_MODULE_VERSIONS,

	BP1048_COMMAND_RESET_PARAMETERS,
};

const char* const cg_bp1048_command[] = {
	"SetParameters",
	"GetParameters",

	"SetADCMode",
	"SetADCMute",
	"SetADCVolume",
	"SetADCSampleRate",
	"SetADCLRSwap",
	"SetADCDCBlockerCoefficient",
	"SetADCFadeTime",
	"SetADCMCLKSource",
	"SetADCDCBlockerEnable",

	"SetAGCMode",
	"SetAGCMaxLevel",
	"SetAGCTargetLevel",
	"SetAGCMaxGain",
	"SetAGCMinGain",
	"SetAGCGainOffset",
	"SetAGCFrameTime",
	"SetAGCHoldNFrameTime",
	"SetAGCAttackTime",
	"SetAGCDecayTime",
	"SetAGCNoiseGateEnable",
	"SetAGCNoiseGateThreshold",
	"SetAGCNoiseGateMode",
	"SetAGCNoiseHoldNFrameTime",

	"SetAutoTuneEnable",
	"SetAutoTuneKey",
	"SetAutoTuneSnap",
	"SetAutoTunePitchAccuracy",

	"SetEchoEnable",
	"SetEchoCutoffFreq",
	"SetEchoAttenuation",
	"SetEchoDelay",
	"SetEchoDirect",
	"SetEchoMaxDelay",

	"SetNoiseSuppressorEnable",
	"SetNoiseSuppressorThreshold",
	"SetNoiseSuppressorRatio",
	"SetNoiseSuppressorAttack",
	"SetNoiseSuppressorRelease",

	"SetHowlingCtrlEnable",
	"SetHowlingCtrlMode",

	"SetSilenceDetectorEnable",
	"SetSilenceDetectorAmplitude",

	"SetFreqShifterEnable",
	"SetFreqShifterDelta",

	"SetPitchShifterEnable",
	"SetPitchShifterKey",

	"SetPitchShifterProEnable",
	"SetPitchShifterProKey",

	"SetReverbEnable",
	"SetReverbDry",
	"SetReverbWet",
	"SetReverbWidth",
	"SetReverbRoom",
	"SetReverbDamping",
	"SetReverbMono",

	"SetReverbPlateEnable",
	"SetReverbPlateFreq",
	"SetReverbPlateModulation",
	"SetReverbPlatePredelay",
	"SetReverbPlateDiffusion",
	"SetReverbPlateDecay",
	"SetReverbPlateDamping",
	"SetReverbPlateWetDryMix",

	"SetReverbProEnable",
	"SetReverbProDry",
	"SetReverbProWet",
	"SetReverbProErWet",
	"SetReverbProErFactor",
	"SetReverbProErWidth",
	"SetReverbProErTolate",
	"SetReverbProRt60",
	"SetReverbProDelay",
	"SetReverbProWidth",
	"SetReverbProWander",
	"SetReverbProSpin",
	"SetReverbProInputLPF",
	"SetReverbProDampLPF",
	"SetReverbProBassLPF",
	"SetReverbProBassB",
	"SetReverbProOutputLPF",

	"SetMV3DEnable",
	"SetMV3DIntensity",

	"SetMV3DPlusEnable",
	"SetMV3DPlusIntensity",

	"SetMVBassEnable",
	"SetMVBassCutoffFreq",
	"SetMVBassIntensity",
	"SetMVBassEnhanced",

	"SetMVBassClassicEnable",
	"SetMVBassClassicCutoffFreq",
	"SetMVBassClassicIntensity",

	"SetVoiceChangerEnable",
	"SetVoiceChangerPitch",
	"SetVoiceChangerFormant",
		
	"SetVoiceChangerProEnable",
	"SetVoiceChangerProPitch",
	"SetVoiceChangerProFormant",

	"SetVocalCutEnable",
	"SetVocalCutWetDryMix",

	"SetVocalRemoverEnable",
	"SetVocalRemoverLowerFreq",
	"SetVocalRemoverHigherFreq",

	"SetPhaseCtrlEnable",
	"SetPhaseCtrlPhaseDifference",

	"SetPCMDelayEnable",
	"SetPCMDelayTime",
	"SetPCMDelayMaxDelay",
	"SetPCMDelayHighQualityEnable",

	"SetHarmonicExciterEnable",
	"SetHarmonicExciterCutoffFreq",
	"SetHarmonicExciterDry",
	"SetHarmonicExciterWet",

	"SetPingPongEnable",
	"SetPingPongAttenuation",
	"SetPingPongDelay",
	"SetPingPongHighQualityEnable",
	"SetPingPongWetDryMix",
	"SetPingPongMaxDelay",

	"SetChorusEnable",
	"SetChorusDelay",
	"SetChorusModulationDepth",
	"SetChorusModulationRate",
	"SetChorusFeedback",
	"SetChorusDry",
	"SetChorusWet",

	"SetAutoWahEnable",
	"SetAutoWahModulationRate",
	"SetAutoWahMinFreq",
	"SetAutoWahMaxFreq",
	"SetAutoWahDepth",
	"SetAutoWahDry",
	"SetAutoWahWet",

	"SetStereoWidenerEnable",
	"SetStereoWidenerShaping",

	"SetGainEnable",
	"SetGainMute",
	"SetGain",

	"SetDRCEnable",
	"SetDRCCrossoverFreq",
	"SetDRCMode",
	"SetDRCQ",
	"SetDRCThreshold",
	"SetDRCRatio",
	"SetDRCAttack",
	"SetDRCRelease",
	"SetDRCPregain1",
	"SetDRCPregain2",

	"SetEQEnable",
	"SetEQPregain",
	"SetEQFilterEnable",
	"SetEQFilterType",
	"SetEQFilterFreq",
	"SetEQFilterQ",
	"SetEQFilterBoost",

	"GetAnalogOutputMode",
	"SetAnalogOutputMode",

	"GetGainLimit",
	"SetGainLimit",

	"NotifySytemStatus",
	"NotifyADCParameters",
	"NotifyAGCParameters",
	"NotifyAutoTuneParameters",
	"NotifyEchoParameters",
	"NotifyNoiseSuppressorParameters",
	"NotifyFreqShifterParameters",
	"NotifyHowlingCtrlParameters",
	"NotifyPitchShifterParameters",
	"NotifyReverbParameters",
	"NotifySilenceDetectorParameters",
	"NotifyMV3DParameters",
	"NotifyMVBassParameters",
	"NotifyVoiceChangerParameters",
	"NotifyVocalCutParameters",
	"NotifyReverbPlateParameters",
	"NotifyReverbProParameters",
	"NotifyVoiceChangerProParameters",
	"NotifyPhaseCtrlParameters",
	"NotifyVocalRemoverParameters",
	"NotifyPitchShifterProParameters",
	"NotifyMVBassClassicParameters",
	"NotifyPCMDelayParameters",
	"NotifyHarmonicExciterParameters",
	"NotifyChorusParameters",
	"NotifyAutoWahParameters",
	"NotifyStereoWidenerParameters",
	"NotifyPingPongParameters",
	"NotifyMV3DPlusParameters",
	"NotifyGainParameters",
	"NotifyDRCParameters",
	"NotifyEQParameters",

	"SetRmsDRCEnable",
	"SetRmsDRCAttack",
	"SetRmsDRCRelease",
	"SetRmsDRCGain",
	"SetRmsDRCNumberPoint",
	"SetRmsDRCPoint",
	"SetRMSDRCModuleID",
	"SaveRMSDRCModule",

	"NotifyRMSDRCParameters",

	"GetSystemAudioSource",
	"SetSystemAudioSource",

	"GetFMFreqBand",
	"GetFMChannelSpace",
	"GetFMSoundChannelMode",
	"SetFMSoundChannelMode",
	"GetFMBassBoost",
	"SetFMBassBoost",
	"GetFMDeEmphasis",
	"GetFMRdsEn",
	"FMAutoSearch",
	"FMManualSearch",
	"SetFMChannelIndex",
	"GetFMChannelIndex",
	"SetFMFreq",
	"GetFMFreq",
	"GetFMState",
	"GetFMStoreCount",
	"GetFMStoreList",

	"NotifySystemAudioSource",
	"NotifyFMFreqBand",
	"NotifyFMChannelSpace",
	"NotifyFMSoundChannelMode",
	"NotifyFMBassBoost",
	"NotifyFMDeEmphasis",
	"NotifyFMRdsEn",
	"NotifyFMChannelIndex",
	"NotifyFMFreq",
	"NotifyFMState",
	"NotifyFMStoreCount",
	"NotifyFMStoreList",

	"SetAimyEQEnable",
	"SetAimyEQPregain",
	"SetAimyEQFilterEnable",
	"SetAimyEQFilterType",
	"SetAimyEQFilterFreq",
	"SetAimyEQFilterQ",
	"SetAimyEQFilterBoost",

	"NotifyAimyEQParameters",

	"SetAimyPCMDelayEnable",
	"SetAimyPCMDelayLeft",
	"SetAimyPCMDelayRight",
	"SetAimyPCMDelayMaxDelay",

	"SetAimyPhaseCtrlEnable",
	"SetAimyPhaseCtrlPhaseDifference",
	"SetAimyPhaseCtrlChannel",

	"NotifyAimyPCMDelayParameters",
	"NotifyAimyPhaseCtrlParameters",

	"SaveParameters",
	"NotifySaveParameters",

	"GetEffectModuleVersions",
	"ResetParameters",
};
/******************** end 模块BP1048 *********************/

}}//namespace VDProtocol::Command