#pragma once

#include <stdint.h>
#include <queue>
#include <mutex>
#include <thread>
#include <assert.h>
#include "Effector.h"
#include "RemoteControl.h"
#include "ConsoleMinikV0.h"
#include "MicrophoneU.h"
#include "Bluetooth.h"
#include "Module5G8.h"
#include "PanelMusicBox.h"

namespace VirtualDevice
{

class LocalSocket;

class Machine : public Effector, public RemoteControl, public ConsoleMinikV0
			, public MicrophoneU, public Bluetooth, public Module5G8, public PanelMusicBox
{
public:
	inline Machine();
	virtual ~Machine();

	inline static Machine* Get();
	virtual bool Open(const char *path_local_socket = nullptr);
	virtual void Close();

	void Register();
	void QueryDevices();
	void QueryVirtualDevice();

	int Write(const void* p_data, size_t length) const;

protected:
	struct DeviceInfo {
		enum {
			SUCCESS = 0,
			REPLY_FORMAT,			//设备应答解析错误
		} status;
		char product_id[0x20];
		char version[16];
		char date[16];
	};
	virtual void NotifyQueryDevice(const DeviceInfo *p_notify_device) {}
	virtual void NotifyQueryVirtualDevice(const char *version) {}

private:
	void NotifyRead(const void* p_data, size_t length);
	void DispatchCommand(const void* p_stream, size_t length);
	void ThreadNotify();

private:
	volatile enum {
		IDLE,
		RUNNING,
	} m_status;

	struct NotifyElement {
		uint16_t command;
		uint8_t data[0x160];
	};
	std::queue<NotifyElement> m_queue_notify;
	std::mutex m_lock_queue_notify;
	std::thread m_thread_notify;

	LocalSocket *mp_client;

	static const char* const scm_path_service;
	static const char* const scm_path_client_default;

	static Machine *smp_this;
};

Machine::Machine() : mp_client(nullptr), m_status(IDLE)
{
	assert(!smp_this);
	smp_this = this;
}

Machine*
Machine::Get()
{
	return smp_this;
}

}//namespace VirtualDevice