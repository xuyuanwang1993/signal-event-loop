

#include "LocalSocket.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include<errno.h>
namespace VirtualDevice
{

LocalSocket::~LocalSocket()
{
	m_state = 0;
	if (m_socket != -1) {
		shutdown(m_socket, SHUT_RDWR);
		close(m_socket);
	}

	unlink(m_sun_path);

	if (m_thread_read.joinable())
		m_thread_read.join();
}

bool
LocalSocket::Listen()
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd == -1)
		return false;

	unlink(m_sun_path);

	sockaddr_un src_addr;
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.sun_family = AF_UNIX;
	strncpy(src_addr.sun_path, m_sun_path, sizeof(src_addr.sun_path) - 1);

	if(bind(fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) == -1) {
		perror("bind failed:");
		close(fd);
		return false;
	}

	m_socket = fd;
	m_state = 1;
	m_thread_read = std::thread(&LocalSocket::ThreadRead, this);
	return true;
}

ssize_t
LocalSocket::Write(const void* p_data, size_t data_size) const
{
	if (m_addr_remote.sun_path[0])
		return sendto(m_socket, p_data, data_size, 0, (struct sockaddr*)&m_addr_remote, sizeof(m_addr_remote));
	return -1;
}

void
LocalSocket::ThreadRead()
{
	sockaddr_un addr_remote;
	socklen_t len = sizeof(addr_remote);
    uint8_t buffer[32*1024];

	while (m_state) {
		ssize_t read_bytes = recvfrom(m_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr_remote, &len);
		if (read_bytes == -1) {
			perror("recvfrom error:");
			continue;
		}

		if (mpf_notify_read)
			mpf_notify_read(buffer, read_bytes);
	}
}

}//namespace VirtualDevice
