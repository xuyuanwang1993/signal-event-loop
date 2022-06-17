#ifndef SERIAL_UTIL_H
#define SERIAL_UTIL_H


#include <stdint.h>
#include <termios.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include "log/aimy-log.h"
// API
extern int aserial_open(const char* dev);
extern int aserial_set_opt(int fd, int baudrate, const uint8_t data_bits, const char parity, const uint8_t stop_bits);
extern ssize_t aserial_read(int fd, void *buf, size_t len, uint32_t timeoutMsec=500);
extern ssize_t aserial_write(int fd, const void *buf, size_t len);
extern void aserial_close(int fd) ;
extern void aserial_set_rts(int fd,bool flag);

#endif // SERIAL_UTIL_H
