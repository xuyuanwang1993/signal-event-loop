#pragma once

#include <sys/types.h>
#include <thread>
#include <functional>

#ifdef _XOPEN_SOURCE
#	include <sys/socket.h>
#	include <sys/un.h>
#else
#	error	"This os is not supported"
#endif//_XOPEN_SOURCE

namespace VirtualDevice
{

class LocalSocket
{
public:
	enum class Error
	{
		SUCCESS = 0,
		EXIST,
	};

	typedef std::function<void(const void *p_data, size_t length)> FuncNotifyRead;

public:
	inline explicit LocalSocket(const char *sun_path, const char *remote_sun_path, FuncNotifyRead pf_notify_read);
	virtual ~LocalSocket();

	bool Listen();
	ssize_t Write(const void *p_data, size_t data_size) const;//return -1:出错; >=0:发出的数据长度;

protected:
	void ThreadRead();

protected:
	volatile unsigned m_state;
	std::thread m_thread_read;

	FuncNotifyRead mpf_notify_read;

	char m_sun_path[sizeof(sockaddr_un::sun_path)];
	int m_socket;
	sockaddr_un m_addr_remote;
};

LocalSocket::LocalSocket(const char *sun_path, const char *remote_sun_path, FuncNotifyRead pf_notify_read)
	: m_state(0), mpf_notify_read(pf_notify_read), m_socket(-1)
{
	strncpy(m_sun_path, sun_path, sizeof(m_sun_path) - 1);

	memset(&m_addr_remote, 0, sizeof(m_addr_remote));
	m_addr_remote.sun_family = AF_UNIX;
	strncpy(m_addr_remote.sun_path, remote_sun_path, sizeof(m_addr_remote.sun_path) - 1);
}

}//namespace VirtualDevice