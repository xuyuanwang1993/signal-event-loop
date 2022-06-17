#ifndef PLATFORM_UTIL_HPP
#define PLATFORM_UTIL_HPP
#include <endian.h>
#if defined(__linux) || defined(__linux__)
#define ENV_LINUX
#include <arpa/inet.h>
#include <errno.h>
#elif defined(WIN32) || defined(_WIN32)
#define ENV_WINDOWS
#define bzero(a,b) memset(a,0,b)
#define FD_SETSIZE      1024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#else
#endif
#include<cstdint>
namespace aimy {
namespace platform {
inline int getErrno()
{
#if defined(__linux) || defined(__linux__)
    return errno;
#elif   defined(WIN32) || defined(_WIN32)
    return WSAGetLastError();
#else
    return 0;
#endif
}

inline uint16_t localToBe16( uint16_t host)
{
    return htobe16(host);
}

inline uint16_t beToLocal16( uint16_t network)
{
    return be16toh(network);
}

inline uint32_t localToBe32( uint32_t host)
{
    return htobe32(host);
}

inline uint32_t beToLocal32( uint32_t network)
{
    return be32toh(network);
}

inline uint64_t localToBe64( uint64_t host)
{
    return htobe64(host);
}

inline uint64_t beToLocal64( uint64_t network)
{
    return be64toh(network);
}

}
}
#endif // PLATFORM_UTIL_HPP
