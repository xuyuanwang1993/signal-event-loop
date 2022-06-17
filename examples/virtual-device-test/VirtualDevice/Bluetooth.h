#pragma once

namespace VirtualDevice
{

class Bluetooth
{
public:
	bool PowerOn() const;
	bool QueryName() const;
	bool Conference() const;
	bool Pair() const;
	bool PairCancel() const;
	bool PairAccept() const;
	bool GetDeviceStatus() const;
	bool BluetoothCommand(const char *str_command) const;

	virtual void NotifyReply(const char *message) {}

protected:
	bool DispatchNotify(unsigned command, const void *p_notify);

private:
	static const char* const scm_cmd_power_on;
	static const char* const scm_cmd_query_name;
	static const char* const scm_cmd_conference;
	static const char* const scm_cmd_pair;
	static const char* const scm_cmd_pair_cancel;
	static const char* const scm_cmd_pair_accept;
	static const char* const scm_cmd_device_status;
};

}//namespace VirtualDevice