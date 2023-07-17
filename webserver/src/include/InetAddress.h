#ifndef _INETADDRESS_H
#define _INETADDRESS_H
#include <arpa/inet.h>

class InetAddress
{
public:
    sockaddr_in _addr;
    socklen_t _addr_len;
    InetAddress(/* args */);
    InetAddress(uint16_t port, const char* ip);
    ~InetAddress();

    void SetAddr(struct sockaddr_in addr);
    const char* GetIP();
    uint16_t GetPort();
};

#endif  // _INETADDRESS_H