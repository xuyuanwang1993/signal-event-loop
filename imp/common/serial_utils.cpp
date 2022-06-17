#include "serial_utils.h"
int aserial_open(const char* dev)
{
    auto fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    return fd;
}

int aserial_set_opt(int fd, int baudrate, const uint8_t data_bits, const char parity, const uint8_t stop_bits)
{
    int dev = fd;
    struct termios opts;
    memset(&opts, 0, sizeof(opts));

    opts.c_cflag |= CLOCAL | CREAD;
    opts.c_cflag &= ~CSIZE;

    switch (data_bits)
    {
    case 5: { opts.c_cflag |= CS5; break; }
    case 6: { opts.c_cflag |= CS6; break; }
    case 7: { opts.c_cflag |= CS7; break; }
    case 8: { opts.c_cflag |= CS8; break; }
    default: { opts.c_cflag |= CS8; break; }
    }

    switch (parity)
    {
    case 'N': { opts.c_cflag &= ~PARENB; break; }
    case 'O': { opts.c_cflag |= PARENB; opts.c_cflag |= PARODD; opts.c_cflag |= (INPCK | ISTRIP); break; }
    case 'E': { opts.c_cflag |= (PARENB | INPCK); opts.c_cflag |= PARENB; opts.c_cflag &= ~PARODD; break; }
    default: { return -3; }
    }

    int baudrate_def;
    switch (baudrate)
    {case 9600: { baudrate_def = B9600; break; }
    case 38400: { baudrate_def = B38400; break; }
    case 57600: { baudrate_def = B57600; break; }
    case 115200: { baudrate_def = B115200; break; }
    case 230400: { baudrate_def = B230400; break; }
    case 460800: { baudrate_def = B460800; break; }
    case 500000: { baudrate_def = B500000; break; }
    case 576000: { baudrate_def = B576000; break; }
    case 921600: { baudrate_def = B921600; break; }
    case 1000000: { baudrate_def = B1000000; break; }
    case 1152000: { baudrate_def = B1152000; break; }
    case 1500000: { baudrate_def = B1500000; break; }
    case 2000000: { baudrate_def = B2000000; break; }
    case 2500000: { baudrate_def = B2500000; break; }
    case 3000000: { baudrate_def = B3000000; break; }
    case 3500000: { baudrate_def = B3500000; break; }
    case 4000000: { baudrate_def = B4000000; break; }
    default: { baudrate_def = B57600; break; }
    }
    cfsetispeed(&opts, baudrate_def);
    cfsetospeed(&opts, baudrate_def);

    switch (stop_bits)
    {
    case 1: { opts.c_cflag &= ~CSTOPB; break; }
    case 2: { opts.c_cflag |= CSTOPB; break; }
    default:{ return -4; }
    }

    opts.c_cc[VTIME] = 1;
    opts.c_cc[VMIN] = 1;
    tcflush(dev, TCIFLUSH);
    if(tcsetattr(dev, TCSANOW, &opts) != 0)
        return -5;
    return 0;
}

ssize_t aserial_read(int fd, void *buf, size_t len,uint32_t timeoutMsec)
{
    if(!buf || !len || fd < 0) return -1;

    struct timeval tv;
    tv.tv_sec = timeoutMsec/1000;
    tv.tv_usec = (timeoutMsec%1000)*1000;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd,&rset);
    int ret = select(fd + 1, &rset, NULL, NULL, &tv);
    if(ret > 0 && FD_ISSET(fd, &rset))
    {
        memset(buf, 0, len);
        ssize_t rlen = read(fd, buf, len);
        if(rlen <= 0)
        {
            switch (errno)
            {
            case ECONNRESET:
            case EIO:
            case EBUSY:
            case EINVAL: { rlen = -1; break; }
            default: { rlen = -2; break;}
            }
            AIMY_ERROR("[aserial_read] read serial data failed.%s",strerror(errno));
        }
        else
        {
            return rlen;
        }
    }
    else if (ret < -0) AIMY_ERROR("[aserial_read] read serial data timeout.%s",strerror(errno));
    return ret;
}

ssize_t aserial_write(int fd, const void *buf, size_t len)
{
    int dev = fd;
    if(dev < 0)
        return -1;
    ssize_t wlen = write(dev, buf, len);

    do
    {
        if(wlen > 0) break;
        switch (errno)
        {
        case ECONNRESET:
        case EIO:
        case EBUSY:
        case EINVAL: { wlen = -1; break; }
        default: { wlen = -2; break; }
        }
        AIMY_ERROR("[aserial_write] failed to write serial data.%s",strerror(errno));
    } while(0);

    return wlen;
}

void aserial_close(int fd)
{
    if(fd >= 0) close(fd);
}

void aserial_set_rts(int fd, bool flag)
{
    int status;
    ioctl(fd,TIOCMGET,&status);
    if(flag)
    {
        status|=TIOCM_RTS;
    }
    else {
        status&=~TIOCM_RTS;
    }
    auto ret=ioctl(fd,TIOCMSET,&status);
    if(ret<0){
        AIMY_ERROR("[aserial_close] failed to set serial rts with ioctl.%s",strerror(errno));
    }
    ioctl(fd,TIOCMGET,&status);
}
